#include "server/game_server.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace cruce::server {

void GameRepository::save_active_match(const Match& match) {
  active_matches_[match.id()] = to_record(match);
}

void GameRepository::save_completed_match(const Match& match) {
  completed_matches_.push_back(to_record(match));
  active_matches_.erase(match.id());
}

void GameRepository::discard_active_match(const std::string& match_id) {
  active_matches_.erase(match_id);
}

const std::vector<MatchRecord>& GameRepository::completed_matches() const {
  return completed_matches_;
}

std::optional<MatchRecord> GameRepository::active_match_record(
    const std::string& match_id) const {
  const auto found = active_matches_.find(match_id);
  if (found == active_matches_.end()) {
    return std::nullopt;
  }
  return found->second;
}

MatchRecord GameRepository::to_record(const Match& match) {
  MatchRecord record;
  record.match_id = match.id();
  record.target_score = match.target_score();
  record.final_score = match.score_board().scores();
  record.winner = match.winner();
  record.round_results = match.round_results();

  for (const auto& player : match.players()) {
    record.participants.push_back(player.id());
  }
  for (const auto& team : match.teams()) {
    record.teams.push_back(team.owner_id());
  }

  return record;
}

Room::Room(std::string room_code, int player_count, int target_score)
    : room_code_(std::move(room_code)),
      player_count_(player_count),
      target_score_(target_score) {
  if (player_count_ < 2 || player_count_ > 4) {
    throw std::invalid_argument("Room player count must be 2, 3, or 4.");
  }
  if (!is_valid_target_score(target_score_)) {
    throw std::invalid_argument("Target score must be 6, 11, or 21.");
  }
}

const std::string& Room::room_code() const {
  return room_code_;
}

int Room::player_count() const {
  return player_count_;
}

int Room::target_score() const {
  return target_score_;
}

const std::vector<Player>& Room::players() const {
  return players_;
}

const std::optional<std::string>& Room::match_id() const {
  return match_id_;
}

ValidationResult Room::add_player(const JoinRequest& request) {
  if (match_id_.has_value()) {
    return ValidationResult::failure("Room already started a match.");
  }
  if (players_.size() >= static_cast<std::size_t>(player_count_)) {
    return ValidationResult::failure("Room is full.");
  }
  const auto duplicate = std::find_if(players_.begin(), players_.end(), [&](const Player& player) {
    return player.id() == request.player_id;
  });
  if (duplicate != players_.end()) {
    return ValidationResult::failure("Player is already in this room.");
  }

  players_.push_back(Player{
      request.player_id,
      request.display_name,
      request.platform,
      static_cast<int>(players_.size()),
  });
  return ValidationResult::success("Player joined room.");
}

bool Room::can_start_match() const {
  return !match_id_.has_value() &&
         players_.size() == static_cast<std::size_t>(player_count_);
}

Match Room::start_match(std::string match_id, std::uint32_t shuffle_seed) {
  if (!can_start_match()) {
    throw std::logic_error("Room is not ready to start.");
  }

  match_id_ = match_id;
  Match match{*match_id_, players_, target_score_};
  const auto started = match.start_round(shuffle_seed);
  if (!started) {
    throw std::logic_error(started.message);
  }
  return match;
}

std::string GameServer::create_room(int player_count, int target_score) {
  const auto code = next_room_code();
  rooms_.emplace(code, Room{code, player_count, target_score});
  return code;
}

ValidationResult GameServer::join_room(
    const std::string& room_code,
    const JoinRequest& request) {
  const auto room_it = rooms_.find(room_code);
  if (room_it == rooms_.end()) {
    return ValidationResult::failure("Room does not exist.");
  }

  auto& room = room_it->second;
  const auto joined = room.add_player(request);
  if (!joined) {
    return joined;
  }

  if (room.can_start_match()) {
    const auto match_id = next_match_id();
    Match match = room.start_match(match_id, next_shuffle_seed_++);
    for (const auto& player : match.players()) {
      player_match_ids_[player.id()] = match.id();
    }
    save_match_state(match);
    active_matches_.emplace(match.id(), std::move(match));
  }

  return joined;
}

ValidationResult GameServer::reconnect_player(const PlayerId& player_id) {
  auto* match = active_match_for(player_id);
  if (match == nullptr) {
    return ValidationResult::failure("Player has no active match.");
  }
  auto* player = match->find_player(player_id);
  if (player == nullptr) {
    return ValidationResult::failure("Unknown player.");
  }
  player->set_connected(true);
  save_match_state(*match);
  return ValidationResult::success("Player reconnected.");
}

ValidationResult GameServer::disconnect_player(const PlayerId& player_id) {
  auto* match = active_match_for(player_id);
  if (match == nullptr) {
    return ValidationResult::failure("Player has no active match.");
  }
  auto* player = match->find_player(player_id);
  if (player == nullptr) {
    return ValidationResult::failure("Unknown player.");
  }
  player->set_connected(false);
  save_match_state(*match);
  return ValidationResult::success("Player disconnected.");
}

ValidationResult GameServer::submit_bid(const PlayerId& player_id, int bid_value) {
  auto* match = active_match_for(player_id);
  if (match == nullptr) {
    return ValidationResult::failure("Player has no active match.");
  }

  const auto result = match->submit_bid(player_id, bid_value);
  if (result) {
    save_match_state(*match);
  }
  return result;
}

ValidationResult GameServer::choose_trump(const PlayerId& player_id, Suit suit) {
  auto* match = active_match_for(player_id);
  if (match == nullptr) {
    return ValidationResult::failure("Player has no active match.");
  }

  const auto result = match->choose_trump(player_id, suit);
  if (result) {
    save_match_state(*match);
  }
  return result;
}

ValidationResult GameServer::submit_card_play(
    const PlayerId& player_id,
    const Card& card) {
  auto* match = active_match_for(player_id);
  if (match == nullptr) {
    return ValidationResult::failure("Player has no active match.");
  }

  const auto before_round_count = match->round_results().size();
  const auto result = match->play_card(player_id, card);
  if (result) {
    if (match->status() == MatchStatus::Complete &&
        match->round_results().size() > before_round_count) {
      repository_.save_completed_match(*match);
    } else {
      save_match_state(*match);
    }
  }
  return result;
}

std::optional<GameSnapshot> GameServer::snapshot_for(const PlayerId& player_id) const {
  const auto match = active_match_for(player_id);
  if (match == nullptr) {
    return std::nullopt;
  }
  return PrivacyFilter::private_state_for(*match, player_id);
}

const Match* GameServer::active_match_for(const PlayerId& player_id) const {
  const auto match_id = player_match_ids_.find(player_id);
  if (match_id == player_match_ids_.end()) {
    return nullptr;
  }

  const auto match = active_matches_.find(match_id->second);
  return match == active_matches_.end() ? nullptr : &match->second;
}

Match* GameServer::active_match_for(const PlayerId& player_id) {
  const auto match_id = player_match_ids_.find(player_id);
  if (match_id == player_match_ids_.end()) {
    return nullptr;
  }

  auto match = active_matches_.find(match_id->second);
  return match == active_matches_.end() ? nullptr : &match->second;
}

bool GameServer::release_completed_match(const std::string& match_id) {
  const auto match = active_matches_.find(match_id);
  if (match == active_matches_.end() || match->second.status() != MatchStatus::Complete) {
    return false;
  }

  for (const auto& player : match->second.players()) {
    player_match_ids_.erase(player.id());
  }
  active_matches_.erase(match);
  return true;
}

bool GameServer::cancel_match(const std::string& match_id) {
  const auto match = active_matches_.find(match_id);
  if (match == active_matches_.end()) {
    return false;
  }

  for (const auto& player : match->second.players()) {
    player_match_ids_.erase(player.id());
  }
  repository_.discard_active_match(match_id);
  active_matches_.erase(match);
  return true;
}

const GameRepository& GameServer::repository() const {
  return repository_;
}

std::string GameServer::next_room_code() {
  return "ROOM-" + std::to_string(next_room_number_++);
}

std::string GameServer::next_match_id() {
  return "match-" + std::to_string(next_match_number_++);
}

void GameServer::save_match_state(Match& match) {
  if (match.status() == MatchStatus::Complete) {
    repository_.save_completed_match(match);
  } else {
    repository_.save_active_match(match);
  }
}

}  // namespace cruce::server
