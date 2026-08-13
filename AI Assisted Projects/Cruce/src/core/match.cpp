#include "core/match.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace cruce {

Player::Player(
    PlayerId id,
    std::string display_name,
    ClientPlatform platform,
    int seat_index)
    : id_(std::move(id)),
      display_name_(std::move(display_name)),
      platform_(platform),
      seat_index_(seat_index) {}

const PlayerId& Player::id() const {
  return id_;
}

const std::string& Player::display_name() const {
  return display_name_;
}

ClientPlatform Player::platform() const {
  return platform_;
}

int Player::seat_index() const {
  return seat_index_;
}

bool Player::connected() const {
  return connected_;
}

void Player::set_connected(bool connected) {
  connected_ = connected;
}

void Player::clear_hand() {
  hand_.clear();
}

void Player::receive_card(const Card& card) {
  hand_.push_back(card);
}

void Player::receive_cards(const std::vector<Card>& cards) {
  hand_.insert(hand_.end(), cards.begin(), cards.end());
}

bool Player::has_card(const Card& card) const {
  return std::find(hand_.begin(), hand_.end(), card) != hand_.end();
}

bool Player::remove_card(const Card& card) {
  const auto found = std::find(hand_.begin(), hand_.end(), card);
  if (found == hand_.end()) {
    return false;
  }
  hand_.erase(found);
  return true;
}

const std::vector<Card>& Player::hand() const {
  return hand_;
}

Team::Team(OwnerId owner_id, std::vector<PlayerId> player_ids)
    : owner_id_(std::move(owner_id)), player_ids_(std::move(player_ids)) {}

const OwnerId& Team::owner_id() const {
  return owner_id_;
}

const std::vector<PlayerId>& Team::player_ids() const {
  return player_ids_;
}

bool Team::contains(const PlayerId& player_id) const {
  return std::find(player_ids_.begin(), player_ids_.end(), player_id) !=
         player_ids_.end();
}

Trick::Trick(int leader_seat) : leader_seat_(leader_seat) {}

int Trick::leader_seat() const {
  return leader_seat_;
}

const std::vector<PlayedCard>& Trick::played_cards() const {
  return played_cards_;
}

void Trick::add_card(const PlayedCard& played_card) {
  played_cards_.push_back(played_card);
}

PlayedCard Trick::winner(Suit trump_suit) const {
  return RulesEngine::resolve_trick_winner(played_cards_, trump_suit);
}

int Trick::points() const {
  return RulesEngine::trick_points(played_cards_);
}

bool Trick::complete(int player_count) const {
  return static_cast<int>(played_cards_.size()) == player_count;
}

Round::Round(int round_number, int player_count, int dealer_seat)
    : round_number_(round_number),
      player_count_(player_count),
      dealer_seat_(dealer_seat),
      leader_seat_((dealer_seat + 1) % player_count) {
  for (int offset = 1; offset <= player_count; ++offset) {
    bidding_order_.push_back((dealer_seat + offset) % player_count);
  }
}

int Round::round_number() const {
  return round_number_;
}

int Round::player_count() const {
  return player_count_;
}

int Round::dealer_seat() const {
  return dealer_seat_;
}

int Round::leader_seat() const {
  return leader_seat_;
}

const std::optional<Suit>& Round::trump_suit() const {
  return trump_suit_;
}

const std::optional<Bid>& Round::winning_bid() const {
  return winning_bid_;
}

const std::vector<Bid>& Round::bids() const {
  return bids_;
}

const Trick& Round::current_trick() const {
  return current_trick_;
}

const std::vector<Trick>& Round::completed_tricks() const {
  return completed_tricks_;
}

const std::map<OwnerId, int>& Round::card_points() const {
  return card_points_;
}

const std::vector<Announcement>& Round::announcements() const {
  return announcements_;
}

const Deck& Round::deck() const {
  return deck_;
}

bool Round::draw_pile_empty() const {
  return deck_.empty();
}

bool Round::bidding_complete() const {
  return next_bid_index_ >= bidding_order_.size();
}

bool Round::first_lead_must_be_trump() const {
  return first_lead_must_be_trump_;
}

ScoreBoard::ScoreBoard(const std::vector<OwnerId>& owners) {
  for (const auto& owner : owners) {
    scores_[owner] = 0;
  }
}

void ScoreBoard::add_round_delta(const std::map<OwnerId, int>& delta_by_owner) {
  for (const auto& [owner, delta] : delta_by_owner) {
    scores_[owner] += delta;
  }
}

void ScoreBoard::add_round_score(const OwnerId& owner_id, int points) {
  scores_[owner_id] += points;
}

void ScoreBoard::apply_failed_bid(const OwnerId& owner_id, int bid_value) {
  scores_[owner_id] -= bid_value;
}

std::optional<OwnerId> ScoreBoard::leader() const {
  if (scores_.empty()) {
    return std::nullopt;
  }

  auto best = scores_.begin();
  bool tied = false;
  for (auto current = std::next(scores_.begin()); current != scores_.end(); ++current) {
    if (current->second > best->second) {
      best = current;
      tied = false;
    } else if (current->second == best->second) {
      tied = true;
    }
  }

  if (tied) {
    return std::nullopt;
  }
  return best->first;
}

std::optional<OwnerId> ScoreBoard::winner(int target_score) const {
  return RulesEngine::winning_owner(scores_, target_score);
}

const std::map<OwnerId, int>& ScoreBoard::scores() const {
  return scores_;
}

Match::Match(std::string match_id, std::vector<Player> players, int target_score)
    : match_id_(std::move(match_id)),
      target_score_(target_score),
      players_(std::move(players)) {
  if (players_.size() < 2 || players_.size() > 4) {
    throw std::invalid_argument("A match requires 2, 3, or 4 players.");
  }
  if (!is_valid_target_score(target_score_)) {
    throw std::invalid_argument("Target score must be 6, 11, or 21.");
  }
  initialize_owners();
}

ValidationResult Match::start_round(std::uint32_t shuffle_seed) {
  if (status_ == MatchStatus::Complete) {
    return ValidationResult::failure("Cannot start a round after the match is complete.");
  }

  active_round_.emplace(
      static_cast<int>(round_results_.size()) + 1,
      static_cast<int>(players_.size()),
      dealer_seat_);
  auto& round = *active_round_;
  round.deck_ = Deck::standard_24();
  round.deck_.shuffle(shuffle_seed);

  for (auto& player : players_) {
    player.clear_hand();
    player.receive_cards(round.deck_.deal(static_cast<std::size_t>(cards_per_player())));
  }

  initialize_round_points();
  status_ = MatchStatus::Bidding;
  return ValidationResult::success("Round started.");
}

ValidationResult Match::submit_bid(const PlayerId& player_id, int bid_value) {
  if (status_ != MatchStatus::Bidding || !active_round_.has_value()) {
    return ValidationResult::failure("The match is not accepting bids.");
  }

  auto& round = *active_round_;
  const auto expected_player = current_turn_player();
  if (!expected_player.has_value() || *expected_player != player_id) {
    return ValidationResult::failure("It is not this player's turn to bid.");
  }

  const bool pass = bid_value == 0;
  const auto validation =
      RulesEngine::validate_bid(round.winning_bid_, bid_value, pass);
  if (!validation) {
    return validation;
  }

  const Bid bid{player_id, bid_value, pass};
  round.bids_.push_back(bid);
  if (!pass) {
    round.winning_bid_ = bid;
  }
  ++round.next_bid_index_;

  if (round.bidding_complete()) {
    if (!round.winning_bid_.has_value()) {
      dealer_seat_ = next_seat_after(dealer_seat_);
      return start_round(next_shuffle_seed_++);
    }
    status_ = MatchStatus::ChoosingTrump;
  }

  return ValidationResult::success(pass ? "Bid passed." : "Bid accepted.");
}

ValidationResult Match::choose_trump(const PlayerId& player_id, Suit suit) {
  if (status_ != MatchStatus::ChoosingTrump || !active_round_.has_value()) {
    return ValidationResult::failure("The match is not choosing trump.");
  }

  auto& round = *active_round_;
  if (!round.winning_bid_.has_value() || round.winning_bid_->player_id != player_id) {
    return ValidationResult::failure("Only the highest bidder can choose trump.");
  }

  round.trump_suit_ = suit;
  round.leader_seat_ = seat_for_player(player_id);
  round.current_trick_ = Trick{round.leader_seat_};
  round.first_lead_must_be_trump_ = true;
  status_ = MatchStatus::Playing;
  return ValidationResult::success("Trump selected.");
}

ValidationResult Match::play_card(const PlayerId& player_id, const Card& card) {
  if (status_ != MatchStatus::Playing || !active_round_.has_value()) {
    return ValidationResult::failure("The match is not accepting card plays.");
  }

  auto& round = *active_round_;
  if (!round.trump_suit_.has_value()) {
    return ValidationResult::failure("Trump has not been selected.");
  }

  const auto expected_player = current_turn_player();
  if (!expected_player.has_value() || *expected_player != player_id) {
    return ValidationResult::failure("It is not this player's turn to play.");
  }

  auto* player = find_player(player_id);
  if (player == nullptr) {
    return ValidationResult::failure("Unknown player.");
  }

  const TrickContext context{
      static_cast<int>(players_.size()),
      *round.trump_suit_,
      round.current_trick_.played_cards(),
      round.draw_pile_empty(),
      round.first_lead_must_be_trump_ && round.current_trick_.played_cards().empty(),
  };
  const auto validation = RulesEngine::validate_card_play(player->hand(), context, card);
  if (!validation) {
    return validation;
  }

  const Card played_card = card;
  maybe_score_announcement(*player, played_card);
  if (!player->remove_card(played_card)) {
    return ValidationResult::failure("Card is not in the player's hand.");
  }

  round.current_trick_.add_card(
      PlayedCard{player_id, player->seat_index(), played_card});

  if (round.current_trick_.complete(static_cast<int>(players_.size()))) {
    complete_current_trick();
  }

  return ValidationResult::success("Card played.");
}

const std::string& Match::id() const {
  return match_id_;
}

int Match::target_score() const {
  return target_score_;
}

int Match::dealer_seat() const {
  return dealer_seat_;
}

MatchStatus Match::status() const {
  return status_;
}

const std::vector<Player>& Match::players() const {
  return players_;
}

const std::vector<Team>& Match::teams() const {
  return teams_;
}

const ScoreBoard& Match::score_board() const {
  return score_board_;
}

const std::vector<RoundResult>& Match::round_results() const {
  return round_results_;
}

const std::optional<OwnerId>& Match::winner() const {
  return winner_;
}

const Round* Match::active_round() const {
  return active_round_ ? &*active_round_ : nullptr;
}

std::optional<PlayerId> Match::current_turn_player() const {
  if (!active_round_.has_value()) {
    return std::nullopt;
  }

  const auto& round = *active_round_;
  if (status_ == MatchStatus::Bidding) {
    if (round.next_bid_index_ >= round.bidding_order_.size()) {
      return std::nullopt;
    }
    return player_at_seat(round.bidding_order_[round.next_bid_index_]).id();
  }

  if (status_ == MatchStatus::ChoosingTrump && round.winning_bid_.has_value()) {
    return round.winning_bid_->player_id;
  }

  if (status_ == MatchStatus::Playing) {
    const int offset = static_cast<int>(round.current_trick_.played_cards().size());
    return player_at_seat((round.leader_seat_ + offset) % static_cast<int>(players_.size())).id();
  }

  return std::nullopt;
}

std::vector<Card> Match::legal_cards_for(const PlayerId& player_id) const {
  std::vector<Card> legal;
  if (status_ != MatchStatus::Playing || !active_round_.has_value()) {
    return legal;
  }

  const auto expected_player = current_turn_player();
  if (!expected_player.has_value() || *expected_player != player_id) {
    return legal;
  }

  const auto* player = find_player(player_id);
  if (player == nullptr) {
    return legal;
  }

  const auto& round = *active_round_;
  if (!round.trump_suit_.has_value()) {
    return legal;
  }

  const TrickContext context{
      static_cast<int>(players_.size()),
      *round.trump_suit_,
      round.current_trick_.played_cards(),
      round.draw_pile_empty(),
      round.first_lead_must_be_trump_ && round.current_trick_.played_cards().empty(),
  };

  for (const auto& card : player->hand()) {
    if (RulesEngine::validate_card_play(player->hand(), context, card).ok) {
      legal.push_back(card);
    }
  }
  return legal;
}

const Player* Match::find_player(const PlayerId& player_id) const {
  const auto found = std::find_if(players_.begin(), players_.end(), [&](const Player& player) {
    return player.id() == player_id;
  });
  return found == players_.end() ? nullptr : &*found;
}

Player* Match::find_player(const PlayerId& player_id) {
  const auto found = std::find_if(players_.begin(), players_.end(), [&](const Player& player) {
    return player.id() == player_id;
  });
  return found == players_.end() ? nullptr : &*found;
}

OwnerId Match::owner_for_player(const PlayerId& player_id) const {
  if (teams_.empty()) {
    return player_id;
  }

  for (const auto& team : teams_) {
    if (team.contains(player_id)) {
      return team.owner_id();
    }
  }

  throw std::invalid_argument("unknown player owner");
}

Player& Match::player_at_seat(int seat_index) {
  for (auto& player : players_) {
    if (player.seat_index() == seat_index) {
      return player;
    }
  }
  throw std::out_of_range("invalid player seat");
}

const Player& Match::player_at_seat(int seat_index) const {
  for (const auto& player : players_) {
    if (player.seat_index() == seat_index) {
      return player;
    }
  }
  throw std::out_of_range("invalid player seat");
}

int Match::seat_for_player(const PlayerId& player_id) const {
  const auto* player = find_player(player_id);
  if (player == nullptr) {
    throw std::invalid_argument("unknown player");
  }
  return player->seat_index();
}

int Match::next_seat_after(int seat_index) const {
  return (seat_index + 1) % static_cast<int>(players_.size());
}

int Match::cards_per_player() const {
  switch (players_.size()) {
    case 2:
      return 8;
    case 3:
      return 8;
    case 4:
      return 6;
    default:
      throw std::logic_error("unsupported player count");
  }
}

bool Match::all_hands_empty() const {
  return std::all_of(players_.begin(), players_.end(), [](const Player& player) {
    return player.hand().empty();
  });
}

void Match::initialize_owners() {
  std::vector<OwnerId> owners;
  if (players_.size() == 4) {
    teams_.push_back(Team{"team-0", {player_at_seat(0).id(), player_at_seat(2).id()}});
    teams_.push_back(Team{"team-1", {player_at_seat(1).id(), player_at_seat(3).id()}});
    owners.push_back("team-0");
    owners.push_back("team-1");
  } else {
    for (const auto& player : players_) {
      owners.push_back(player.id());
    }
  }
  score_board_ = ScoreBoard{owners};
}

void Match::initialize_round_points() {
  auto& round = *active_round_;
  round.card_points_.clear();
  if (teams_.empty()) {
    for (const auto& player : players_) {
      round.card_points_[player.id()] = 0;
    }
  } else {
    for (const auto& team : teams_) {
      round.card_points_[team.owner_id()] = 0;
    }
  }
}

void Match::complete_current_trick() {
  auto& round = *active_round_;
  const auto trick_winner = round.current_trick_.winner(*round.trump_suit_);
  const auto owner = owner_for_player(trick_winner.player_id);
  round.card_points_[owner] += round.current_trick_.points();
  round.completed_tricks_.push_back(round.current_trick_);
  round.leader_seat_ = trick_winner.seat_index;

  if (players_.size() == 2 && !round.deck_.empty()) {
    player_at_seat(trick_winner.seat_index).receive_card(*round.deck_.draw());
    const int other_seat = next_seat_after(trick_winner.seat_index);
    if (!round.deck_.empty()) {
      player_at_seat(other_seat).receive_card(*round.deck_.draw());
    }
  }

  round.current_trick_ = Trick{round.leader_seat_};
  round.first_lead_must_be_trump_ = false;

  if (round.deck_.empty() && all_hands_empty()) {
    finish_round();
  }
}

void Match::finish_round() {
  auto& round = *active_round_;
  const auto bidder_owner = owner_for_player(round.winning_bid_->player_id);
  const auto score =
      RulesEngine::score_round(round.card_points_, bidder_owner, round.winning_bid_->value);
  score_board_.add_round_delta(score.match_point_delta);

  RoundResult result;
  result.round_number = round.round_number_;
  result.bid_winner = round.winning_bid_->player_id;
  result.bidder_owner = bidder_owner;
  result.bid_value = round.winning_bid_->value;
  result.trump_suit = *round.trump_suit_;
  result.card_points = score.card_points;
  result.match_point_delta = score.match_point_delta;
  result.bid_succeeded = score.bid_succeeded;
  result.announcements = round.announcements_;
  round_results_.push_back(result);

  winner_ = score_board_.winner(target_score_);
  if (winner_.has_value()) {
    status_ = MatchStatus::Complete;
    active_round_.reset();
    return;
  }

  dealer_seat_ = next_seat_after(dealer_seat_);
  start_round(next_shuffle_seed_++);
}

void Match::maybe_score_announcement(Player& player, const Card& card) {
  auto& round = *active_round_;
  if (round.announced_suits_.contains(card.suit)) {
    return;
  }
  if (card.rank != Rank::King && card.rank != Rank::Queen) {
    return;
  }

  const Rank pair_rank = card.rank == Rank::King ? Rank::Queen : Rank::King;
  if (!player.has_card(Card{card.suit, pair_rank})) {
    return;
  }

  const int points = RulesEngine::announcement_points(card.suit, *round.trump_suit_);
  round.announced_suits_.insert(card.suit);
  round.announcements_.push_back(Announcement{player.id(), card.suit, points});
  round.card_points_[owner_for_player(player.id())] += points;
}

}  // namespace cruce
