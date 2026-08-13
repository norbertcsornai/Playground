#include "core/deck.hpp"
#include "core/match.hpp"
#include "core/rules_engine.hpp"

#include "test_support.hpp"

#include <map>
#include <set>
#include <vector>

using namespace cruce;

namespace {

std::vector<Player> make_players(int count) {
  std::vector<Player> players;
  for (int index = 0; index < count; ++index) {
    const auto number = std::to_string(index);
    players.push_back(Player{"p" + number, "Player " + number, ClientPlatform::Bot, index});
  }
  return players;
}

void deck_has_24_unique_cards_and_120_points() {
  const auto cards = RulesEngine::create_deck();
  expect_eq(cards.size(), std::size_t{24}, "deck should contain 24 cards");

  std::set<int> unique_ids;
  int point_total = 0;
  for (const auto& card : cards) {
    unique_ids.insert(card.id());
    point_total += card.point_value();
  }

  expect_eq(unique_ids.size(), std::size_t{24}, "deck should not contain duplicates");
  expect_eq(point_total, 120, "deck card points should total 120");
}

void dealing_counts_match_player_mode() {
  for (const auto player_count : {2, 3, 4}) {
    Match match{"match-" + std::to_string(player_count), make_players(player_count), 6};
    const auto started = match.start_round(123);
    expect(started.ok, "round should start");
    const auto* round = match.active_round();
    expect(round != nullptr, "round should be active");

    const auto expected_hand = player_count == 4 ? std::size_t{6} : std::size_t{8};
    for (const auto& player : match.players()) {
      expect_eq(player.hand().size(), expected_hand, "hand size should match mode");
    }

    const auto expected_draw_pile = player_count == 2 ? std::size_t{8} : std::size_t{0};
    expect_eq(round->deck().size(), expected_draw_pile, "draw pile size should match mode");
  }
}

void bid_validation_enforces_range_and_increase() {
  std::optional<Bid> no_bid;
  expect(RulesEngine::validate_bid(no_bid, 1, false).ok, "first bid of 1 is valid");
  expect(RulesEngine::validate_bid(no_bid, 0, true).ok, "passing is valid");
  expect(!RulesEngine::validate_bid(no_bid, 5, false).ok, "bid above 4 is invalid");

  const Bid current{"p1", 2, false};
  expect(!RulesEngine::validate_bid(current, 2, false).ok, "equal bid is invalid");
  expect(RulesEngine::validate_bid(current, 3, false).ok, "higher bid is valid");
}

void card_play_must_follow_suit_and_win_when_possible() {
  const std::vector<Card> hand{
      Card{Suit::Hearts, Rank::Ace},
      Card{Suit::Hearts, Rank::Nine},
      Card{Suit::Spades, Rank::Ace},
  };
  const TrickContext context{
      4,
      Suit::Spades,
      {PlayedCard{"p0", 0, Card{Suit::Hearts, Rank::Ten}}},
      true,
      false,
  };

  expect(!RulesEngine::validate_card_play(hand, context, Card{Suit::Spades, Rank::Ace}).ok,
         "player must follow led suit");
  expect(!RulesEngine::validate_card_play(hand, context, Card{Suit::Hearts, Rank::Nine}).ok,
         "player must win with the Ace when possible");
  expect(RulesEngine::validate_card_play(hand, context, Card{Suit::Hearts, Rank::Ace}).ok,
         "highest legal heart should be valid");
}

void card_play_must_trump_when_unable_to_follow_suit() {
  const std::vector<Card> hand{
      Card{Suit::Spades, Rank::Nine},
      Card{Suit::Clubs, Rank::Ace},
  };
  const TrickContext context{
      4,
      Suit::Spades,
      {PlayedCard{"p0", 0, Card{Suit::Hearts, Rank::Ace}}},
      true,
      false,
  };

  expect(!RulesEngine::validate_card_play(hand, context, Card{Suit::Clubs, Rank::Ace}).ok,
         "player must trump when unable to follow suit");
  expect(RulesEngine::validate_card_play(hand, context, Card{Suit::Spades, Rank::Nine}).ok,
         "trump is valid when led suit is missing");
}

void two_player_draw_pile_relaxes_card_rules() {
  const std::vector<Card> hand{
      Card{Suit::Hearts, Rank::Nine},
      Card{Suit::Clubs, Rank::Ace},
  };
  const TrickContext relaxed{
      2,
      Suit::Spades,
      {PlayedCard{"p0", 0, Card{Suit::Hearts, Rank::Ace}}},
      false,
      false,
  };
  const TrickContext strict{
      2,
      Suit::Spades,
      {PlayedCard{"p0", 0, Card{Suit::Hearts, Rank::Ace}}},
      true,
      false,
  };

  expect(RulesEngine::validate_card_play(hand, relaxed, Card{Suit::Clubs, Rank::Ace}).ok,
         "2-player mode allows any card while draw pile has cards");
  expect(!RulesEngine::validate_card_play(hand, strict, Card{Suit::Clubs, Rank::Ace}).ok,
         "2-player mode becomes strict after draw pile is empty");
}

void trump_resolves_trick_winner() {
  const std::vector<PlayedCard> played{
      PlayedCard{"p0", 0, Card{Suit::Hearts, Rank::Ace}},
      PlayedCard{"p1", 1, Card{Suit::Spades, Rank::Nine}},
      PlayedCard{"p2", 2, Card{Suit::Spades, Rank::Ten}},
      PlayedCard{"p3", 3, Card{Suit::Hearts, Rank::Ten}},
  };

  const auto winner = RulesEngine::resolve_trick_winner(played, Suit::Spades);
  expect_eq(winner.player_id, std::string{"p2"}, "highest trump should win");
}

void round_scoring_applies_bid_success_and_failure() {
  const auto failed = RulesEngine::score_round(
      std::map<OwnerId, int>{{"team-0", 65}, {"team-1", 55}},
      "team-0",
      2);
  expect(!failed.bid_succeeded, "65 card points should fail a bid of 2");
  expect_eq(failed.match_point_delta.at("team-0"), -2, "failed bidder loses bid value");
  expect_eq(failed.match_point_delta.at("team-1"), 1, "non-bidder still scores earned points");

  const auto succeeded = RulesEngine::score_round(
      std::map<OwnerId, int>{{"team-0", 99}, {"team-1", 21}},
      "team-0",
      2);
  expect(succeeded.bid_succeeded, "99 card points should make a bid of 2");
  expect_eq(succeeded.match_point_delta.at("team-0"), 3, "successful bidder gets earned points");
  expect_eq(succeeded.match_point_delta.at("team-1"), 0, "21 points earns no match points");
}

void target_score_requires_untied_leader() {
  expect(!RulesEngine::winning_owner({{"p1", 6}, {"p2", 6}}, 6).has_value(),
         "tie at target should continue");
  const auto winner = RulesEngine::winning_owner({{"p1", 7}, {"p2", 6}}, 6);
  expect(winner.has_value(), "untied score above target should win");
  expect_eq(*winner, std::string{"p1"}, "p1 should be the winner");
}

void match_bidding_and_trump_flow() {
  Match match{"match-flow", make_players(3), 6};
  expect(match.start_round(99).ok, "round should start");
  expect_eq(*match.current_turn_player(), std::string{"p1"}, "seat after dealer bids first");

  expect(match.submit_bid("p1", 1).ok, "p1 can bid 1");
  expect(match.submit_bid("p2", 0).ok, "p2 can pass");
  expect(match.submit_bid("p0", 2).ok, "dealer can bid last");
  expect(match.status() == MatchStatus::ChoosingTrump, "highest bidder should choose trump");

  const auto* bidder = match.find_player("p0");
  expect(bidder != nullptr, "bidder should exist");
  const auto trump = bidder->hand().front().suit;
  expect(match.choose_trump("p0", trump).ok, "bidder should choose trump");
  expect(match.status() == MatchStatus::Playing, "match should enter playing state");
  expect_eq(*match.current_turn_player(), std::string{"p0"}, "bid winner should lead");
}

void completed_two_player_trick_returns_turn_to_trick_winner() {
  for (std::uint32_t seed = 1; seed < 200; ++seed) {
    Match match{"match-two-player-trick", make_players(2), 6};
    expect(match.start_round(seed).ok, "round should start");
    expect(match.submit_bid("p1", 1).ok, "p1 bids first");
    expect(match.submit_bid("p0", 0).ok, "p0 passes");

    const auto* leader = match.find_player("p1");
    const auto* responder = match.find_player("p0");
    expect(leader != nullptr, "leader should exist");
    expect(responder != nullptr, "responder should exist");

    for (const auto suit : all_suits()) {
      for (const auto& lead_card : leader->hand()) {
        if (lead_card.suit != suit) {
          continue;
        }
        for (const auto& response_card : responder->hand()) {
          if (response_card.suit == suit &&
              RulesEngine::card_beats(response_card, lead_card, suit, suit)) {
            expect(match.choose_trump("p1", suit).ok, "leader chooses trump");
            expect(match.play_card("p1", lead_card).ok, "leader plays first trump");
            expect_eq(*match.current_turn_player(), std::string{"p0"}, "responder acts second");
            expect(match.play_card("p0", response_card).ok, "responder wins the trick");

            const auto* round = match.active_round();
            expect(round != nullptr, "round should remain active");
            expect_eq(round->current_trick().played_cards().size(), std::size_t{0},
                      "completed trick should clear the table");
            expect_eq(round->completed_tricks().size(), std::size_t{1},
                      "completed trick should be recorded");
            expect_eq(*match.current_turn_player(), std::string{"p0"},
                      "trick winner should lead next");
            return;
          }
        }
      }
    }
  }

  expect(false, "a deterministic seed should produce a responder-winning first trick");
}

void announcement_points_count_when_pair_card_is_played() {
  for (std::uint32_t seed = 1; seed < 500; ++seed) {
    Match match{"match-announcement", make_players(2), 6};
    expect(match.start_round(seed).ok, "round should start");
    expect(match.submit_bid("p1", 1).ok, "p1 bids first");
    expect(match.submit_bid("p0", 0).ok, "p0 passes");

    const auto* bidder = match.find_player("p1");
    expect(bidder != nullptr, "bidder should exist");
    for (const auto suit : all_suits()) {
      const Card king{suit, Rank::King};
      const Card queen{suit, Rank::Queen};
      if (bidder->has_card(king) && bidder->has_card(queen)) {
        expect(match.choose_trump("p1", suit).ok, "bidder chooses paired suit as trump");
        expect(match.play_card("p1", king).ok, "playing one pair card scores announcement");

        const auto* round = match.active_round();
        expect(round != nullptr, "round should remain active");
        expect_eq(round->card_points().at("p1"), 40,
                  "trump announcement should score immediately");
        return;
      }
    }
  }

  expect(false, "a deterministic seed should give the bidder a King/Queen pair");
}

}  // namespace

int main() {
  int failures = 0;
  failures += run_test("deck_has_24_unique_cards_and_120_points", deck_has_24_unique_cards_and_120_points);
  failures += run_test("dealing_counts_match_player_mode", dealing_counts_match_player_mode);
  failures += run_test("bid_validation_enforces_range_and_increase", bid_validation_enforces_range_and_increase);
  failures += run_test("card_play_must_follow_suit_and_win_when_possible", card_play_must_follow_suit_and_win_when_possible);
  failures += run_test("card_play_must_trump_when_unable_to_follow_suit", card_play_must_trump_when_unable_to_follow_suit);
  failures += run_test("two_player_draw_pile_relaxes_card_rules", two_player_draw_pile_relaxes_card_rules);
  failures += run_test("trump_resolves_trick_winner", trump_resolves_trick_winner);
  failures += run_test("round_scoring_applies_bid_success_and_failure", round_scoring_applies_bid_success_and_failure);
  failures += run_test("target_score_requires_untied_leader", target_score_requires_untied_leader);
  failures += run_test("match_bidding_and_trump_flow", match_bidding_and_trump_flow);
  failures += run_test(
      "completed_two_player_trick_returns_turn_to_trick_winner",
      completed_two_player_trick_returns_turn_to_trick_winner);
  failures += run_test(
      "announcement_points_count_when_pair_card_is_played",
      announcement_points_count_when_pair_card_is_played);
  return failures == 0 ? 0 : 1;
}
