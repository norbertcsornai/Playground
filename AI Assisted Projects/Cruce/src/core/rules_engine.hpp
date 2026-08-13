#pragma once

#include "core/types.hpp"

#include <map>
#include <optional>
#include <vector>

namespace cruce {

struct TrickContext {
  int player_count = 0;
  Suit trump_suit = Suit::Hearts;
  std::vector<PlayedCard> played_cards;
  bool draw_pile_empty = true;
  bool first_lead_must_be_trump = false;
};

struct RoundScore {
  std::map<OwnerId, int> card_points;
  std::map<OwnerId, int> match_point_delta;
  OwnerId bidder_owner;
  int bid_value = 0;
  bool bid_succeeded = false;
};

class RulesEngine {
 public:
  static std::vector<Card> create_deck();

  static ValidationResult validate_bid(
      const std::optional<Bid>& current_highest_bid,
      int bid_value,
      bool pass);

  static ValidationResult validate_card_play(
      const std::vector<Card>& hand,
      const TrickContext& context,
      const Card& card);

  static bool card_beats(
      const Card& challenger,
      const Card& current_winner,
      Suit led_suit,
      Suit trump_suit);

  static PlayedCard resolve_trick_winner(
      const std::vector<PlayedCard>& played_cards,
      Suit trump_suit);

  static int trick_points(const std::vector<PlayedCard>& played_cards);
  static int announcement_points(Suit announcement_suit, Suit trump_suit);

  static RoundScore score_round(
      const std::map<OwnerId, int>& card_points,
      const OwnerId& bidder_owner,
      int bid_value);

  static std::optional<OwnerId> winning_owner(
      const std::map<OwnerId, int>& match_scores,
      int target_score);
};

}  // namespace cruce
