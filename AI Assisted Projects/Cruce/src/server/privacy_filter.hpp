#pragma once

#include "core/match.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace cruce::server {

struct PlayerSnapshot {
  PlayerId player_id;
  std::string display_name;
  ClientPlatform platform = ClientPlatform::Bot;
  int seat_index = 0;
  bool connected = true;
  std::size_t cards_in_hand = 0;
  OwnerId owner_id;
};

struct GameSnapshot {
  std::string match_id;
  MatchStatus status = MatchStatus::WaitingForPlayers;
  int target_score = 6;
  int dealer_seat = 0;
  std::optional<PlayerId> current_turn_player;
  std::optional<Suit> trump_suit;
  std::vector<PlayerSnapshot> players;
  std::map<OwnerId, int> scores;
  std::map<OwnerId, int> round_points;
  std::vector<Bid> bids;
  std::vector<PlayedCard> current_trick;
  std::vector<PlayedCard> last_completed_trick;
  std::optional<PlayerId> last_trick_winner;
  std::vector<RoundResult> round_results;
  std::vector<Card> own_hand;
  std::vector<int> legal_card_ids;
  std::optional<OwnerId> winner;
};

class PrivacyFilter {
 public:
  static GameSnapshot private_state_for(const Match& match, const PlayerId& player_id);
};

}  // namespace cruce::server
