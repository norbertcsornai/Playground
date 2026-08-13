#pragma once

#include "core/match.hpp"
#include "server/privacy_filter.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace cruce::server {

struct JoinRequest {
  PlayerId player_id;
  std::string display_name;
  ClientPlatform platform = ClientPlatform::Web;
};

struct MatchRecord {
  std::string match_id;
  std::vector<PlayerId> participants;
  std::vector<OwnerId> teams;
  int target_score = 6;
  std::map<OwnerId, int> final_score;
  std::optional<OwnerId> winner;
  std::vector<RoundResult> round_results;
};

class GameRepository {
 public:
  void save_active_match(const Match& match);
  void save_completed_match(const Match& match);
  void discard_active_match(const std::string& match_id);
  const std::vector<MatchRecord>& completed_matches() const;
  std::optional<MatchRecord> active_match_record(const std::string& match_id) const;

 private:
  static MatchRecord to_record(const Match& match);

  std::map<std::string, MatchRecord> active_matches_;
  std::vector<MatchRecord> completed_matches_;
};

class Room {
 public:
  Room() = default;
  Room(std::string room_code, int player_count, int target_score);

  const std::string& room_code() const;
  int player_count() const;
  int target_score() const;
  const std::vector<Player>& players() const;
  const std::optional<std::string>& match_id() const;

  ValidationResult add_player(const JoinRequest& request);
  bool can_start_match() const;
  Match start_match(std::string match_id, std::uint32_t shuffle_seed);

 private:
  std::string room_code_;
  int player_count_ = 0;
  int target_score_ = 6;
  std::vector<Player> players_;
  std::optional<std::string> match_id_;
};

class GameServer {
 public:
  std::string create_room(int player_count, int target_score);
  ValidationResult join_room(const std::string& room_code, const JoinRequest& request);
  ValidationResult reconnect_player(const PlayerId& player_id);
  ValidationResult disconnect_player(const PlayerId& player_id);

  ValidationResult submit_bid(const PlayerId& player_id, int bid_value);
  ValidationResult choose_trump(const PlayerId& player_id, Suit suit);
  ValidationResult submit_card_play(const PlayerId& player_id, const Card& card);

  std::optional<GameSnapshot> snapshot_for(const PlayerId& player_id) const;
  const Match* active_match_for(const PlayerId& player_id) const;
  Match* active_match_for(const PlayerId& player_id);
  bool release_completed_match(const std::string& match_id);
  bool cancel_match(const std::string& match_id);
  const GameRepository& repository() const;

 private:
  std::string next_room_code();
  std::string next_match_id();
  void save_match_state(Match& match);

  int next_room_number_ = 1;
  int next_match_number_ = 1;
  std::uint32_t next_shuffle_seed_ = 42;
  std::map<std::string, Room> rooms_;
  std::map<std::string, Match> active_matches_;
  std::map<PlayerId, std::string> player_match_ids_;
  GameRepository repository_;
};

}  // namespace cruce::server
