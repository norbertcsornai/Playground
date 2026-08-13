#include "core/rules_engine.hpp"

#include "core/deck.hpp"

#include <algorithm>
#include <stdexcept>

namespace cruce {

namespace {

bool hand_contains(const std::vector<Card>& hand, const Card& card) {
  return std::find(hand.begin(), hand.end(), card) != hand.end();
}

bool has_suit(const std::vector<Card>& hand, Suit suit) {
  return std::any_of(hand.begin(), hand.end(), [suit](const Card& card) {
    return card.suit == suit;
  });
}

std::vector<Card> legal_by_suit(
    const std::vector<Card>& hand,
    Suit led_suit,
    Suit trump_suit) {
  std::vector<Card> legal;
  const bool can_follow_led = has_suit(hand, led_suit);
  const bool can_trump = has_suit(hand, trump_suit);

  for (const auto& candidate : hand) {
    if (can_follow_led) {
      if (candidate.suit == led_suit) {
        legal.push_back(candidate);
      }
    } else if (can_trump) {
      if (candidate.suit == trump_suit) {
        legal.push_back(candidate);
      }
    } else {
      legal.push_back(candidate);
    }
  }

  return legal;
}

}  // namespace

std::vector<Card> RulesEngine::create_deck() {
  return Deck::standard_24().cards();
}

ValidationResult RulesEngine::validate_bid(
    const std::optional<Bid>& current_highest_bid,
    int bid_value,
    bool pass) {
  if (pass) {
    return ValidationResult::success();
  }

  if (bid_value < 1 || bid_value > 4) {
    return ValidationResult::failure("Bid must be between 1 and 4, or 0 to pass.");
  }

  if (current_highest_bid.has_value() && bid_value <= current_highest_bid->value) {
    return ValidationResult::failure("Bid must be higher than the current highest bid.");
  }

  return ValidationResult::success();
}

ValidationResult RulesEngine::validate_card_play(
    const std::vector<Card>& hand,
    const TrickContext& context,
    const Card& card) {
  if (!hand_contains(hand, card)) {
    return ValidationResult::failure("Card is not in the player's hand.");
  }

  const bool is_leading = context.played_cards.empty();
  if (is_leading) {
    if (context.first_lead_must_be_trump && card.suit != context.trump_suit) {
      return ValidationResult::failure("The first card of the first trick must be trump.");
    }
    return ValidationResult::success();
  }

  if (context.player_count == 2 && !context.draw_pile_empty) {
    return ValidationResult::success();
  }

  const Suit led_suit = context.played_cards.front().card.suit;
  const auto suit_legal_cards = legal_by_suit(hand, led_suit, context.trump_suit);
  if (std::find(suit_legal_cards.begin(), suit_legal_cards.end(), card) ==
      suit_legal_cards.end()) {
    if (has_suit(hand, led_suit)) {
      return ValidationResult::failure("Player must follow the led suit.");
    }
    if (has_suit(hand, context.trump_suit)) {
      return ValidationResult::failure("Player must play trump when unable to follow suit.");
    }
    return ValidationResult::failure("Card is not legal for this trick.");
  }

  const auto current_winner =
      resolve_trick_winner(context.played_cards, context.trump_suit);
  const bool can_win = std::any_of(
      suit_legal_cards.begin(),
      suit_legal_cards.end(),
      [&](const Card& candidate) {
        return card_beats(candidate, current_winner.card, led_suit, context.trump_suit);
      });

  if (can_win &&
      !card_beats(card, current_winner.card, led_suit, context.trump_suit)) {
    return ValidationResult::failure("Player must play a card that can currently win.");
  }

  return ValidationResult::success();
}

bool RulesEngine::card_beats(
    const Card& challenger,
    const Card& current_winner,
    Suit led_suit,
    Suit trump_suit) {
  if (challenger.suit == current_winner.suit) {
    return challenger.strength() > current_winner.strength();
  }

  if (challenger.suit == trump_suit && current_winner.suit != trump_suit) {
    return true;
  }

  if (current_winner.suit == trump_suit && challenger.suit != trump_suit) {
    return false;
  }

  return challenger.suit == led_suit && current_winner.suit != led_suit;
}

PlayedCard RulesEngine::resolve_trick_winner(
    const std::vector<PlayedCard>& played_cards,
    Suit trump_suit) {
  if (played_cards.empty()) {
    throw std::invalid_argument("cannot resolve an empty trick");
  }

  const Suit led_suit = played_cards.front().card.suit;
  PlayedCard winner = played_cards.front();
  for (std::size_t index = 1; index < played_cards.size(); ++index) {
    const auto& candidate = played_cards[index];
    if (card_beats(candidate.card, winner.card, led_suit, trump_suit)) {
      winner = candidate;
    }
  }
  return winner;
}

int RulesEngine::trick_points(const std::vector<PlayedCard>& played_cards) {
  int total = 0;
  for (const auto& played : played_cards) {
    total += played.card.point_value();
  }
  return total;
}

int RulesEngine::announcement_points(Suit announcement_suit, Suit trump_suit) {
  return announcement_suit == trump_suit ? 40 : 20;
}

RoundScore RulesEngine::score_round(
    const std::map<OwnerId, int>& card_points,
    const OwnerId& bidder_owner,
    int bid_value) {
  RoundScore score;
  score.card_points = card_points;
  score.bidder_owner = bidder_owner;
  score.bid_value = bid_value;

  for (const auto& [owner, points] : card_points) {
    const int earned_match_points = points / 33;
    if (owner == bidder_owner) {
      score.bid_succeeded = earned_match_points >= bid_value;
      score.match_point_delta[owner] =
          score.bid_succeeded ? earned_match_points : -bid_value;
    } else {
      score.match_point_delta[owner] = earned_match_points;
    }
  }

  if (!score.match_point_delta.contains(bidder_owner)) {
    score.bid_succeeded = false;
    score.match_point_delta[bidder_owner] = -bid_value;
  }

  return score;
}

std::optional<OwnerId> RulesEngine::winning_owner(
    const std::map<OwnerId, int>& match_scores,
    int target_score) {
  std::optional<OwnerId> best_owner;
  int best_score = target_score - 1;
  bool tied_for_best = false;

  for (const auto& [owner, score] : match_scores) {
    if (score < target_score) {
      continue;
    }
    if (!best_owner.has_value() || score > best_score) {
      best_owner = owner;
      best_score = score;
      tied_for_best = false;
    } else if (score == best_score) {
      tied_for_best = true;
    }
  }

  if (tied_for_best) {
    return std::nullopt;
  }
  return best_owner;
}

}  // namespace cruce
