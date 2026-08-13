#define WIN32_LEAN_AND_MEAN

#include "core/rules_engine.hpp"
#include "core/types.hpp"
#include "server/game_server.hpp"
#include "server/user_store.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

namespace {

std::atomic_bool keep_running = true;

struct Invitation {
  int id = 0;
  std::string from;
  std::string to;
  int player_count = 2;
  std::optional<int> waiting_room_id;
  std::string status = "pending";
  std::optional<std::string> match_id;
};

struct TargetSelection {
  int id = 0;
  int player_count = 2;
  std::vector<std::string> players;
  std::string chooser;
  std::string first_bidder;
};

struct ChatMessage {
  std::string from;
  std::string message;
};

struct WaitingRoom {
  int id = 0;
  int player_count = 3;
  std::vector<std::string> players;
  std::vector<ChatMessage> messages;
};

struct RematchOffer {
  int id = 0;
  int player_count = 2;
  std::vector<std::string> players;
  std::string winner;
  std::map<std::string, bool> accepted_by_player;
};

struct MatchHistoryEntry {
  std::string match_id;
  std::string completed_at;
  std::string winner;
  int target_score = 0;
  std::map<std::string, int> final_score;
  std::vector<std::string> players;
  std::vector<std::string> winning_players;
  int rounds = 0;
};

struct PlayerProfile {
  std::string username;
  std::string display_name;
  std::string avatar_initial;
  std::string avatar_color;
  int total_wins = 0;
  std::vector<MatchHistoryEntry> recent_matches;
};

void handle_signal(int) {
  keep_running = false;
}

std::string escape_json(const std::string& text) {
  std::string escaped;
  for (const char ch : text) {
    switch (ch) {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        break;
      default:
        escaped += ch;
        break;
    }
  }
  return escaped;
}

std::string json_string(const std::string& text) {
  return "\"" + escape_json(text) + "\"";
}

std::string utc_timestamp() {
  const auto now = std::chrono::system_clock::now();
  const auto time = std::chrono::system_clock::to_time_t(now);
  std::tm utc{};
#ifdef _WIN32
  gmtime_s(&utc, &time);
#else
  gmtime_r(&time, &utc);
#endif
  std::ostringstream out;
  out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
  return out.str();
}

std::vector<std::string> split_text(const std::string& text, char delimiter) {
  std::vector<std::string> values;
  std::string item;
  std::istringstream input(text);
  while (std::getline(input, item, delimiter)) {
    if (!item.empty()) {
      values.push_back(item);
    }
  }
  return values;
}

std::string lower_ascii(std::string text) {
  for (auto& ch : text) {
    if (ch >= 'A' && ch <= 'Z') {
      ch = static_cast<char>(ch - 'A' + 'a');
    }
  }
  return text;
}

std::string card_image_path(const cruce::Card& card) {
  return "/assets/cards/" + lower_ascii(cruce::to_string(card.rank)) + "_" +
         lower_ascii(cruce::to_string(card.suit)) + ".png";
}

std::string card_json(const cruce::Card& card) {
  std::ostringstream out;
  out << "{\"id\":" << card.id()
      << ",\"label\":" << json_string(card.label())
      << ",\"suit\":" << json_string(cruce::to_string(card.suit))
      << ",\"rank\":" << json_string(cruce::to_string(card.rank))
      << ",\"points\":" << card.point_value()
      << ",\"image\":" << json_string(card_image_path(card)) << "}";
  return out.str();
}

std::string score_map_json(const std::map<cruce::OwnerId, int>& scores) {
  std::ostringstream out;
  out << "{";
  bool first = true;
  for (const auto& [owner, score] : scores) {
    if (!first) {
      out << ",";
    }
    out << json_string(owner) << ":" << score;
    first = false;
  }
  out << "}";
  return out.str();
}

bool contains_text(const std::vector<std::string>& values, const std::string& value) {
  return std::find(values.begin(), values.end(), value) != values.end();
}

std::string string_array_json(const std::vector<std::string>& values) {
  std::ostringstream out;
  out << "[";
  for (std::size_t index = 0; index < values.size(); ++index) {
    if (index > 0) {
      out << ",";
    }
    out << json_string(values[index]);
  }
  out << "]";
  return out.str();
}

std::string bool_map_json(const std::map<std::string, bool>& values) {
  std::ostringstream out;
  out << "{";
  bool first = true;
  for (const auto& [key, value] : values) {
    if (!first) {
      out << ",";
    }
    out << json_string(key) << ":" << (value ? "true" : "false");
    first = false;
  }
  out << "}";
  return out.str();
}

std::string join_text(const std::vector<std::string>& values, const std::string& separator) {
  std::ostringstream out;
  for (std::size_t index = 0; index < values.size(); ++index) {
    if (index > 0) {
      out << separator;
    }
    out << values[index];
  }
  return out.str();
}

bool is_team_owner(const std::string& owner_id) {
  return owner_id.rfind("team-", 0) == 0;
}

std::string owner_display_label(const std::string& owner_id) {
  if (!is_team_owner(owner_id)) {
    return owner_id;
  }

  try {
    return "Team " + std::to_string(std::stoi(owner_id.substr(5)) + 1);
  } catch (const std::exception&) {
    return owner_id;
  }
}

// Shared AI decision helper used by both live lobby bots and offline self-play.
// Keeping the choices in one class prevents training data and playable bot behavior
// from drifting apart.
class CruceHeuristicBot {
 public:
  // Choose the smallest legal bid that beats the current bid and still matches
  // the estimated strength of the bot's private hand.
  static int choose_bid(const cruce::server::GameSnapshot& state) {
    const int highest = highest_bid_value(state);
    const int desired = desired_bid_value(state);
    if (desired <= highest) {
      return 0;
    }
    return (std::min)(4, (std::max)(1, highest + 1));
  }

  // Pick the suit with the best mix of card count, high-card points, card
  // strength, and King-Queen announcement potential.
  static cruce::Suit choose_trump(const cruce::server::GameSnapshot& state) {
    auto best = cruce::Suit::Hearts;
    int best_score = (std::numeric_limits<int>::min)();
    for (const auto suit : cruce::all_suits()) {
      const int score = trump_suit_score(state, suit);
      if (score > best_score) {
        best = suit;
        best_score = score;
      }
    }
    return best;
  }

  static std::optional<cruce::Card> choose_card(
      const cruce::server::GameSnapshot& state) {
    auto legal = legal_cards(state);
    if (legal.empty()) {
      return std::nullopt;
    }

    // When leading a trick, there is no current winner yet, so the bot chooses
    // the strongest lead according to high-card, trump, and announcement value.
    const auto winner = current_trick_winner(state);
    if (!winner.has_value()) {
      return best_card_by(legal, [&](const cruce::Card& card) {
        return lead_score(state, card);
      });
    }

    const auto actor = state.current_turn_player.value_or("");
    const auto actor_owner = owner_for_player(state, actor);
    const bool own_side_winning =
        owner_for_player(state, winner->player_id) == actor_owner;

    // Split legal choices into cards that can overtake the current trick and
    // cards that cannot. The server has already filtered illegal plays.
    std::vector<cruce::Card> winners;
    std::vector<cruce::Card> losers;
    for (const auto& card : legal) {
      if (card_beats_current(state, card)) {
        winners.push_back(card);
      } else {
        losers.push_back(card);
      }
    }

    // If our player or partner is already winning, prefer feeding points without
    // spending a winning card.
    if (own_side_winning && !losers.empty()) {
      return best_card_by(losers, [&](const cruce::Card& card) {
        return feed_partner_score(state, card);
      });
    }

    // If the other side is winning and we can capture the trick, choose the
    // cheapest useful winner, with extra value for tricks worth many points.
    if (!winners.empty()) {
      return best_card_by(winners, [&](const cruce::Card& card) {
        return capture_score(state, card);
      });
    }

    // If every legal card wins while our side is already ahead, still prefer the
    // card that gives our side points and avoids wasting trump.
    if (own_side_winning) {
      return best_card_by(legal, [&](const cruce::Card& card) {
        return feed_partner_score(state, card);
      });
    }

    // With no way to win the trick, discard the least costly card.
    return best_card_by(legal, [&](const cruce::Card& card) {
      return discard_score(state, card);
    });
  }

 private:
  // Compact facts about one suit in the bot's hand. These values feed bidding
  // and trump selection.
  struct SuitProfile {
    int count = 0;
    int points = 0;
    int strength = 0;
    bool ace = false;
    bool ten = false;
    bool king = false;
    bool queen = false;
  };

  // The bidding system allows 1 through 4; 0 means pass/accept.
  static int highest_bid_value(const cruce::server::GameSnapshot& state) {
    int highest = 0;
    for (const auto& bid : state.bids) {
      if (!bid.passed) {
        highest = (std::max)(highest, bid.value);
      }
    }
    return highest;
  }

  // In 4-player games several player ids can share the same owner/team id.
  static std::string owner_for_player(
      const cruce::server::GameSnapshot& state,
      const std::string& player_id) {
    for (const auto& player : state.players) {
      if (player.player_id == player_id) {
        return player.owner_id;
      }
    }
    return player_id;
  }

  // Finds the best opponent score so the bot can bid more aggressively when an
  // opponent is close to the match target.
  static int best_other_score(
      const cruce::server::GameSnapshot& state,
      const std::string& actor_owner) {
    int best = 0;
    for (const auto& [owner, score] : state.scores) {
      if (owner != actor_owner) {
        best = (std::max)(best, score);
      }
    }
    return best;
  }

  // Count points and key ranks for a possible trump suit.
  static SuitProfile profile_for_suit(
      const cruce::server::GameSnapshot& state,
      cruce::Suit suit) {
    SuitProfile profile;
    for (const auto& card : state.own_hand) {
      if (card.suit != suit) {
        continue;
      }
      ++profile.count;
      profile.points += card.point_value();
      profile.strength += card.strength();
      profile.ace = profile.ace || card.rank == cruce::Rank::Ace;
      profile.ten = profile.ten || card.rank == cruce::Rank::Ten;
      profile.king = profile.king || card.rank == cruce::Rank::King;
      profile.queen = profile.queen || card.rank == cruce::Rank::Queen;
    }
    return profile;
  }

  // Higher scores mean this suit is safer to name as trump.
  static int trump_suit_score(
      const cruce::server::GameSnapshot& state,
      cruce::Suit suit) {
    const auto profile = profile_for_suit(state, suit);
    int score = profile.count * 12 + profile.points * 2 + profile.strength * 4;
    if (profile.ace) {
      score += 16;
    }
    if (profile.ten) {
      score += 12;
    }
    if (profile.king && profile.queen) {
      score += 34;
    } else if (profile.king || profile.queen) {
      score += 5;
    }
    return score;
  }

  // Converts private hand strength into a desired bid. Thresholds are tuned by
  // player count because 2-, 3-, and 4-player hands have different risk.
  static int desired_bid_value(const cruce::server::GameSnapshot& state) {
    int hand_points = 0;
    int hand_strength = 0;
    int high_cards = 0;
    int marriage_count = 0;
    int best_trump_score = 0;

    for (const auto& card : state.own_hand) {
      hand_points += card.point_value();
      hand_strength += card.strength();
      if (card.rank == cruce::Rank::Ace || card.rank == cruce::Rank::Ten) {
        ++high_cards;
      }
    }

    for (const auto suit : cruce::all_suits()) {
      const auto profile = profile_for_suit(state, suit);
      if (profile.king && profile.queen) {
        ++marriage_count;
      }
      best_trump_score = (std::max)(best_trump_score, trump_suit_score(state, suit));
    }

    const int player_count = static_cast<int>(state.players.size());
    int potential =
        hand_points + hand_strength * 2 + high_cards * 9 + marriage_count * 14 +
        best_trump_score;

    int bid = 0;
    if (player_count == 3) {
      if (potential >= 220) {
        bid = 4;
      } else if (potential >= 180) {
        bid = 3;
      } else if (potential >= 140) {
        bid = 2;
      } else if (potential >= 95) {
        bid = 1;
      }
    } else if (player_count == 4) {
      if (potential >= 205) {
        bid = 4;
      } else if (potential >= 165) {
        bid = 3;
      } else if (potential >= 125) {
        bid = 2;
      } else if (potential >= 85) {
        bid = 1;
      }
    } else {
      if (potential >= 230) {
        bid = 4;
      } else if (potential >= 185) {
        bid = 3;
      } else if (potential >= 145) {
        bid = 2;
      } else if (potential >= 100) {
        bid = 1;
      }
    }

    const auto actor = state.current_turn_player.value_or("");
    const auto actor_owner = owner_for_player(state, actor);
    const int own_score = state.scores.contains(actor_owner) ? state.scores.at(actor_owner) : 0;
    const int needed_to_win = state.target_score - own_score;
    if (needed_to_win > 0 && needed_to_win <= 4 && bid >= needed_to_win) {
      bid = (std::max)(bid, needed_to_win);
    }
    if (best_other_score(state, actor_owner) >= state.target_score - 1 && bid >= 1) {
      bid = (std::min)(4, bid + 1);
    }
    return bid;
  }

  // Convert server-provided legal card ids back into Card objects from the bot's
  // private hand.
  static std::vector<cruce::Card> legal_cards(
      const cruce::server::GameSnapshot& state) {
    std::set<int> legal_ids(state.legal_card_ids.begin(), state.legal_card_ids.end());
    std::vector<cruce::Card> legal;
    for (const auto& card : state.own_hand) {
      if (legal_ids.contains(card.id())) {
        legal.push_back(card);
      }
    }
    return legal;
  }

  // Reuses the authoritative rule engine to determine who is currently winning
  // the trick.
  static std::optional<cruce::PlayedCard> current_trick_winner(
      const cruce::server::GameSnapshot& state) {
    if (state.current_trick.empty() || !state.trump_suit.has_value()) {
      return std::nullopt;
    }
    return cruce::RulesEngine::resolve_trick_winner(
        state.current_trick,
        *state.trump_suit);
  }

  // Tests whether a candidate legal card would become the new trick winner.
  static bool card_beats_current(
      const cruce::server::GameSnapshot& state,
      const cruce::Card& card) {
    const auto winner = current_trick_winner(state);
    if (!winner.has_value() || !state.trump_suit.has_value()) {
      return false;
    }
    return cruce::RulesEngine::card_beats(
        card,
        winner->card,
        state.current_trick.front().card.suit,
        *state.trump_suit);
  }

  // Current trick points matter because capturing a valuable trick is worth
  // spending a stronger card.
  static int trick_points(const cruce::server::GameSnapshot& state) {
    int points = 0;
    for (const auto& played : state.current_trick) {
      points += played.card.point_value();
    }
    return points;
  }

  // A King or Queen can announce points if the matching pair card is still in
  // the player's hand when it is played.
  static bool has_pair_card(
      const cruce::server::GameSnapshot& state,
      const cruce::Card& card) {
    if (card.rank != cruce::Rank::King && card.rank != cruce::Rank::Queen) {
      return false;
    }
    const auto pair_rank =
        card.rank == cruce::Rank::King ? cruce::Rank::Queen : cruce::Rank::King;
    return std::any_of(
        state.own_hand.begin(),
        state.own_hand.end(),
        [&](const cruce::Card& hand_card) {
          return hand_card.suit == card.suit && hand_card.rank == pair_rank;
        });
  }

  // Announcement points are 40 in trump and 20 in a non-trump suit.
  static int announcement_bonus(
      const cruce::server::GameSnapshot& state,
      const cruce::Card& card) {
    if (!has_pair_card(state, card)) {
      return 0;
    }
    return state.trump_suit.has_value() && card.suit == *state.trump_suit ? 40 : 20;
  }

  // Suit length helps identify safer leads because more cards in the same suit
  // give the bot follow-up options.
  static int cards_in_suit(
      const cruce::server::GameSnapshot& state,
      cruce::Suit suit) {
    return static_cast<int>(std::count_if(
        state.own_hand.begin(),
        state.own_hand.end(),
        [suit](const cruce::Card& card) { return card.suit == suit; }));
  }

  // Cost estimates how painful it is to spend a card. Trump and high-point cards
  // are more expensive unless they also unlock an announcement.
  static int card_cost(
      const cruce::server::GameSnapshot& state,
      const cruce::Card& card) {
    int cost = card.point_value() * 10 + card.strength() * 2;
    if (state.trump_suit.has_value() && card.suit == *state.trump_suit) {
      cost += 24;
    }
    cost -= announcement_bonus(state, card) * 2;
    return cost;
  }

  // Leading prefers strong high cards, longer suits, trump pressure, and
  // announcement opportunities.
  static int lead_score(
      const cruce::server::GameSnapshot& state,
      const cruce::Card& card) {
    int score =
        card.point_value() * 5 + card.strength() * 8 + cards_in_suit(state, card.suit) * 5;
    if (state.trump_suit.has_value() && card.suit == *state.trump_suit) {
      score += 18;
    }
    if (card.rank == cruce::Rank::Ace) {
      score += 12;
    } else if (card.rank == cruce::Rank::Ten) {
      score += 7;
    }
    score += announcement_bonus(state, card) * 3;
    return score;
  }

  // Capturing balances the value already on the table against the cost of the
  // card required to win.
  static int capture_score(
      const cruce::server::GameSnapshot& state,
      const cruce::Card& card) {
    return trick_points(state) * 8 + announcement_bonus(state, card) * 3 -
           card_cost(state, card);
  }

  // Feeding is useful when our side already wins the trick, especially in
  // 4-player team games.
  static int feed_partner_score(
      const cruce::server::GameSnapshot& state,
      const cruce::Card& card) {
    return card.point_value() * 12 + announcement_bonus(state, card) * 3 -
           (state.trump_suit.has_value() && card.suit == *state.trump_suit ? 18 : 0);
  }

  // Discarding chooses the least valuable card when the bot cannot win.
  static int discard_score(
      const cruce::server::GameSnapshot& state,
      const cruce::Card& card) {
    return -card_cost(state, card);
  }

  // Generic helper for choosing the highest-scoring card by one of the scoring
  // functions above.
  template <typename ScoreFn>
  static cruce::Card best_card_by(
      const std::vector<cruce::Card>& cards,
      ScoreFn score_fn) {
    return *std::max_element(
        cards.begin(),
        cards.end(),
        [&](const cruce::Card& left, const cruce::Card& right) {
          const int left_score = score_fn(left);
          const int right_score = score_fn(right);
          if (left_score == right_score) {
            return left.id() > right.id();
          }
          return left_score < right_score;
        });
  }
};

std::string history_entry_json(const MatchHistoryEntry& entry) {
  std::ostringstream out;
  out << "{\"matchId\":" << json_string(entry.match_id)
      << ",\"completedAt\":" << json_string(entry.completed_at)
      << ",\"winner\":" << json_string(owner_display_label(entry.winner))
      << ",\"targetScore\":" << entry.target_score
      << ",\"finalScore\":" << score_map_json(entry.final_score)
      << ",\"players\":" << string_array_json(entry.players)
      << ",\"winningPlayers\":" << string_array_json(entry.winning_players)
      << ",\"rounds\":" << entry.rounds << "}";
  return out.str();
}

std::string profile_json(const PlayerProfile& profile) {
  std::ostringstream out;
  out << "{\"username\":" << json_string(profile.username)
      << ",\"displayName\":" << json_string(profile.display_name)
      << ",\"avatarInitial\":" << json_string(profile.avatar_initial)
      << ",\"avatarColor\":" << json_string(profile.avatar_color)
      << ",\"totalWins\":" << profile.total_wins
      << ",\"recentMatches\":[";
  for (std::size_t index = 0; index < profile.recent_matches.size(); ++index) {
    if (index > 0) {
      out << ",";
    }
    out << history_entry_json(profile.recent_matches[index]);
  }
  out << "]}";
  return out.str();
}

std::string snapshot_json(const cruce::server::GameSnapshot& state) {
  std::ostringstream out;
  out << "{";
  out << "\"matchId\":" << json_string(state.match_id) << ",";
  out << "\"status\":" << json_string(cruce::to_string(state.status)) << ",";
  out << "\"targetScore\":" << state.target_score << ",";
  out << "\"dealerSeat\":" << state.dealer_seat << ",";
  out << "\"currentTurn\":";
  if (state.current_turn_player.has_value()) {
    out << json_string(*state.current_turn_player);
  } else {
    out << "null";
  }
  out << ",\"trump\":";
  if (state.trump_suit.has_value()) {
    out << json_string(cruce::to_string(*state.trump_suit));
  } else {
    out << "null";
  }
  out << ",\"winner\":";
  if (state.winner.has_value()) {
    out << json_string(*state.winner);
  } else {
    out << "null";
  }

  out << ",\"scores\":" << score_map_json(state.scores);
  out << ",\"roundPoints\":" << score_map_json(state.round_points);

  out << ",\"players\":[";
  for (std::size_t index = 0; index < state.players.size(); ++index) {
    const auto& player = state.players[index];
    if (index > 0) {
      out << ",";
    }
    out << "{\"id\":" << json_string(player.player_id)
        << ",\"name\":" << json_string(player.display_name)
        << ",\"platform\":" << json_string(cruce::to_string(player.platform))
        << ",\"seat\":" << player.seat_index
        << ",\"connected\":" << (player.connected ? "true" : "false")
        << ",\"cardsInHand\":" << player.cards_in_hand
        << ",\"owner\":" << json_string(player.owner_id)
        << ",\"team\":";
    if (is_team_owner(player.owner_id)) {
      out << json_string(owner_display_label(player.owner_id));
    } else {
      out << "null";
    }
    out << "}";
  }
  out << "]";

  out << ",\"bids\":[";
  for (std::size_t index = 0; index < state.bids.size(); ++index) {
    const auto& bid = state.bids[index];
    if (index > 0) {
      out << ",";
    }
    out << "{\"player\":" << json_string(bid.player_id)
        << ",\"value\":" << bid.value
        << ",\"passed\":" << (bid.passed ? "true" : "false") << "}";
  }
  out << "]";

  out << ",\"currentTrick\":[";
  for (std::size_t index = 0; index < state.current_trick.size(); ++index) {
    const auto& played = state.current_trick[index];
    if (index > 0) {
      out << ",";
    }
    out << "{\"player\":" << json_string(played.player_id)
        << ",\"seat\":" << played.seat_index
        << ",\"card\":" << card_json(played.card) << "}";
  }
  out << "]";

  out << ",\"lastTrickWinner\":";
  if (state.last_trick_winner.has_value()) {
    out << json_string(*state.last_trick_winner);
  } else {
    out << "null";
  }

  out << ",\"lastCompletedTrick\":[";
  for (std::size_t index = 0; index < state.last_completed_trick.size(); ++index) {
    const auto& played = state.last_completed_trick[index];
    if (index > 0) {
      out << ",";
    }
    out << "{\"player\":" << json_string(played.player_id)
        << ",\"seat\":" << played.seat_index
        << ",\"card\":" << card_json(played.card) << "}";
  }
  out << "]";

  out << ",\"roundResults\":[";
  for (std::size_t index = 0; index < state.round_results.size(); ++index) {
    const auto& result = state.round_results[index];
    if (index > 0) {
      out << ",";
    }
    out << "{\"roundNumber\":" << result.round_number
        << ",\"bidWinner\":" << json_string(result.bid_winner)
        << ",\"bidderOwner\":" << json_string(result.bidder_owner)
        << ",\"bidValue\":" << result.bid_value
        << ",\"trump\":" << json_string(cruce::to_string(result.trump_suit))
        << ",\"bidSucceeded\":" << (result.bid_succeeded ? "true" : "false")
        << ",\"cardPoints\":" << score_map_json(result.card_points)
        << ",\"matchPointDelta\":" << score_map_json(result.match_point_delta)
        << "}";
  }
  out << "]";

  out << ",\"ownHand\":[";
  for (std::size_t index = 0; index < state.own_hand.size(); ++index) {
    if (index > 0) {
      out << ",";
    }
    out << card_json(state.own_hand[index]);
  }
  out << "]";

  out << ",\"legalCardIds\":[";
  for (std::size_t index = 0; index < state.legal_card_ids.size(); ++index) {
    if (index > 0) {
      out << ",";
    }
    out << state.legal_card_ids[index];
  }
  out << "]";
  out << "}";
  return out.str();
}

std::string action_json(
    const cruce::ValidationResult& result,
    const std::optional<cruce::server::GameSnapshot>& state) {
  std::string body = "{\"ok\":" + std::string(result.ok ? "true" : "false") +
                     ",\"message\":" + json_string(result.message);
  if (state.has_value()) {
    body += ",\"state\":" + snapshot_json(*state);
  }
  body += "}";
  return body;
}

std::string url_decode(const std::string& text) {
  std::string decoded;
  for (std::size_t index = 0; index < text.size(); ++index) {
    if (text[index] == '%' && index + 2 < text.size()) {
      const std::string hex = text.substr(index + 1, 2);
      decoded.push_back(static_cast<char>(std::strtoul(hex.c_str(), nullptr, 16)));
      index += 2;
    } else if (text[index] == '+') {
      decoded.push_back(' ');
    } else {
      decoded.push_back(text[index]);
    }
  }
  return decoded;
}

std::map<std::string, std::string> query_params(const std::string& target) {
  std::map<std::string, std::string> params;
  const auto mark = target.find('?');
  if (mark == std::string::npos) {
    return params;
  }

  const std::string query = target.substr(mark + 1);
  std::size_t start = 0;
  while (start <= query.size()) {
    const auto amp = query.find('&', start);
    const auto pair =
        query.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
    const auto equals = pair.find('=');
    if (equals != std::string::npos) {
      params[url_decode(pair.substr(0, equals))] = url_decode(pair.substr(equals + 1));
    }
    if (amp == std::string::npos) {
      break;
    }
    start = amp + 1;
  }
  return params;
}

std::string path_only(const std::string& target) {
  const auto mark = target.find('?');
  return mark == std::string::npos ? target : target.substr(0, mark);
}

cruce::ClientPlatform parse_platform(const std::string& text) {
  if (text == "WindowsDesktop") {
    return cruce::ClientPlatform::WindowsDesktop;
  }
  if (text == "Mobile") {
    return cruce::ClientPlatform::Mobile;
  }
  if (text == "Bot") {
    return cruce::ClientPlatform::Bot;
  }
  return cruce::ClientPlatform::Web;
}

std::optional<cruce::Suit> parse_suit(const std::string& text) {
  if (text == "Hearts") {
    return cruce::Suit::Hearts;
  }
  if (text == "Diamonds") {
    return cruce::Suit::Diamonds;
  }
  if (text == "Clubs") {
    return cruce::Suit::Clubs;
  }
  if (text == "Spades") {
    return cruce::Suit::Spades;
  }
  return std::nullopt;
}

std::optional<cruce::Card> find_own_card(
    const cruce::server::GameSnapshot& state,
    int card_id) {
  const auto found = std::find_if(
      state.own_hand.begin(),
      state.own_hand.end(),
      [card_id](const cruce::Card& card) { return card.id() == card_id; });
  if (found == state.own_hand.end()) {
    return std::nullopt;
  }
  return *found;
}

std::string html_page() {
  return R"HTML(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Cruce Web Client</title>
  <style>
    * { box-sizing: border-box; }
    :root {
      --bg: #f4f7f5;
      --surface: #ffffff;
      --surface-2: #f8faf9;
      --ink: #1b2b28;
      --muted: #5f716c;
      --line: #d7ded9;
      --brand: #153c35;
      --accent: #a84f2b;
      --accent-strong: #843d23;
      --gold: #dfb64c;
      --success: #145331;
      --success-bg: #e2f4e9;
      --waiting: #704b12;
      --waiting-bg: #fff0c4;
      --danger: #9f1d1d;
      --danger-bg: #fde8e8;
    }
    body {
      font-family: Segoe UI, Arial, sans-serif;
      margin: 0;
      background: var(--bg);
      color: var(--ink);
    }
    main { width: 100%; max-width: none; margin: 0; padding: 16px; }
    h1 {
      margin: -16px -16px 18px;
      padding: 18px 22px;
      background: var(--brand);
      color: #fff;
      border-bottom: 4px solid var(--gold);
      font-size: clamp(1.7rem, 3vw, 2.4rem);
      font-weight: 800;
    }
    h2 { margin: 0 0 12px; color: var(--brand); font-size: 1.35rem; }
    h3 { margin: 12px 0 8px; color: var(--brand); font-size: 1rem; }
    .panel {
      background: var(--surface);
      border: 1px solid var(--line);
      border-radius: 8px;
      padding: 16px;
      margin: 12px 0;
      box-shadow: 0 12px 26px rgba(21, 60, 53, 0.08);
    }
    #auth { max-width: 760px; }
    label { display: block; margin: 10px 0; color: var(--brand); font-weight: 700; }
    input, select {
      width: 260px;
      max-width: 100%;
      padding: 10px 11px;
      border: 1px solid var(--line);
      border-radius: 6px;
      background: #fff;
      color: var(--ink);
      font: inherit;
    }
    input:focus, select:focus { outline: 2px solid rgba(168, 79, 43, 0.25); border-color: var(--accent); }
    button {
      margin: 4px 6px 4px 0;
      padding: 9px 13px;
      border: 0;
      border-radius: 6px;
      background: var(--accent);
      color: #fff;
      font: inherit;
      font-weight: 800;
      cursor: pointer;
      box-shadow: 0 2px 0 rgba(0, 0, 0, 0.12);
    }
    button:hover:not(:disabled) { background: var(--accent-strong); }
    button:disabled { background: #cfd9d4; color: #66756f; cursor: default; box-shadow: none; }
    .row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
      padding: 11px 12px;
      border: 1px solid var(--line);
      border-radius: 8px;
      background: var(--surface-2);
      margin: 8px 0;
    }
    .muted { color: var(--muted); }
    .error { color: #9f1d1d; }
    .ok { color: var(--success); }
    code { background: #e8efeb; padding: 2px 4px; border-radius: 4px; }
    .cards { display: flex; flex-wrap: wrap; gap: 10px; margin-top: 8px; width: 100%; }
    .card-button {
      flex: 0 0 104px;
      width: 104px;
      padding: 7px;
      background: #fffdf9;
      border: 1px solid #c9d1cc;
      border-radius: 8px;
      box-shadow: 0 6px 14px rgba(27, 43, 40, 0.08);
      transition: transform 140ms ease, border-color 140ms ease, opacity 140ms ease;
    }
    .card-button:hover:not(:disabled) { transform: translateY(-1px); }
    .card-button.legal-card { border-color: var(--success); box-shadow: 0 0 0 2px rgba(20, 83, 49, 0.18); }
    .card-button.illegal-card { opacity: 0.48; }
    .card-button img { display: block; width: 82px; height: auto; margin: 0 auto 4px; }
    .played-card { display: flex; align-items: center; gap: 8px; margin: 6px 0; padding: 6px; border-radius: 8px; background: #fffdf9; animation: deal-in 180ms ease-out; }
    .played-card img { width: 44px; height: auto; }
    .profile-card { display: grid; grid-template-columns: auto 1fr; gap: 10px; align-items: center; }
    .avatar {
      width: 38px;
      height: 38px;
      border-radius: 50%;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      background: var(--brand);
      color: #fff;
      font-weight: 900;
    }
    .profile-history { grid-column: 1 / -1; color: var(--muted); font-size: 0.93rem; }
    .chat-box { min-height: 92px; max-height: 180px; overflow-y: auto; border: 1px solid var(--line); border-radius: 8px; padding: 10px; background: var(--surface-2); }
    .chat-line { margin: 4px 0; }
    .table-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(210px, 1fr)); gap: 12px; }
    .table-grid > div { background: var(--surface-2); border: 1px solid var(--line); border-radius: 8px; padding: 12px; }
    .seat-map {
      display: grid;
      grid-template-columns: repeat(3, minmax(0, 1fr));
      grid-template-rows: repeat(3, minmax(58px, auto));
      gap: 8px;
      margin: 8px 0 10px;
      min-height: 210px;
      width: 100%;
      overflow: visible;
    }
    .seat {
      border: 1px solid var(--line);
      border-radius: 8px;
      background: #fff;
      padding: 8px;
      min-height: 56px;
      display: flex;
      gap: 8px;
      align-items: center;
      min-width: 0;
      overflow-wrap: anywhere;
      box-shadow: 0 4px 10px rgba(21, 60, 53, 0.06);
    }
    .seat > span { min-width: 0; }
    .seat.active { border-color: var(--success); background: var(--success-bg); animation: turn-pulse 900ms ease-in-out infinite alternate; }
    .seat-0 { grid-column: 2; grid-row: 3; }
    .seat-1 { grid-column: 1; grid-row: 2; }
    .seat-2 { grid-column: 2; grid-row: 1; }
    .seat-3 { grid-column: 3; grid-row: 2; }
    .table-center { grid-column: 2; grid-row: 2; }
    .seat-empty { visibility: hidden; }
    .pill { display: inline-block; padding: 5px 8px; border-radius: 6px; background: #e8efeb; color: var(--brand); margin: 2px; font-weight: 700; }
    .turn-banner { font-size: 1.05rem; font-weight: 800; padding: 11px 12px; border-radius: 8px; border: 1px solid var(--line); }
    .turn-banner.ready { color: var(--success); background: var(--success-bg); border-color: #b9dfc8; }
    .turn-banner.waiting { color: var(--waiting); background: var(--waiting-bg); border-color: #e9ce7f; }
    @keyframes deal-in {
      from { opacity: 0; transform: translateY(12px) scale(0.96); }
      to { opacity: 1; transform: translateY(0) scale(1); }
    }
    @keyframes turn-pulse {
      from { box-shadow: 0 0 0 rgba(20, 83, 49, 0.08); }
      to { box-shadow: 0 0 0 4px rgba(20, 83, 49, 0.13); }
    }
    @media (max-width: 720px) {
      main { padding: 10px; }
      h1 { margin: -10px -10px 12px; padding: 16px; }
      .panel { padding: 12px; }
      .row { align-items: flex-start; flex-direction: column; }
      .table-grid { grid-template-columns: 1fr; }
      .seat-map {
        display: flex;
        flex-direction: column;
        min-height: 0;
      }
      #tableSeats .seat {
        grid-column: auto !important;
        grid-row: auto !important;
        width: auto;
      }
    }
  </style>
</head>
<body>
<main>
  <h1>Cruce</h1>
  <section id="auth" class="panel">
    <h2>Login or Register</h2>
    <label>Username <input id="username" autocomplete="username" placeholder="Enter username"></label>
    <label>Password <input id="password" type="password" autocomplete="current-password" placeholder="Enter password"></label>
    <button onclick="login()">Login</button>
    <button onclick="registerUser()">Register</button>
    <p id="authMessage" class="muted">Welcome back.</p>
  </section>
  <section id="lobby" class="panel" hidden>
    <h2>Online Players</h2>
    <p id="session" class="muted"></p>
    <label id="gameSizeLabel">Invite to
      <select id="gameSize">
        <option value="2">2-player game</option>
        <option value="3">3-player game</option>
        <option value="4">4-player game</option>
      </select>
    </label>
    <div id="online"></div>
    <h3>Your Profile</h3>
    <div id="profile" class="profile-card"></div>
  </section>
  <section id="waiting" class="panel" hidden>
    <h2>Waiting Room</h2>
    <p id="waitingStatus" class="turn-banner waiting"></p>
    <div id="waitingPlayers"></div>
    <h3>Chat</h3>
    <div id="waitingChat" class="chat-box"></div>
    <label>Message <input id="chatInput" maxlength="240"></label>
    <button onclick="sendChat()">Send</button>
    <button onclick="returnToLobby()">Return to Lobby</button>
  </section>
  <section id="rematch" class="panel" hidden>
    <h2>Rematch</h2>
    <p id="rematchNotice" class="turn-banner waiting"></p>
    <div id="rematchResponses" class="muted"></div>
    <button id="rematchYes" onclick="respondRematch(true)">Yes</button>
    <button id="rematchNo" onclick="respondRematch(false)">No</button>
    <button onclick="returnToLobby()">Return to Lobby</button>
  </section>
  <section id="invites" class="panel" hidden>
    <h2>Invitations</h2>
    <div id="incoming"></div>
  </section>
  <section id="target" class="panel" hidden>
    <h2>Target Score</h2>
    <p id="targetNotice" class="turn-banner waiting"></p>
    <div id="targetControls">
      <button onclick="selectTarget(6)">6 points</button>
      <button onclick="selectTarget(11)">11 points</button>
      <button onclick="selectTarget(21)">21 points</button>
    </div>
    <button onclick="returnToLobby()">Return to Lobby</button>
  </section>
  <section id="match" class="panel" hidden>
    <h2>Match</h2>
    <p id="turnNotice" class="turn-banner waiting"></p>
    <p id="matchMessage" class="muted"></p>
    <button onclick="returnToLobby()">Return to Lobby</button>
    <div class="table-grid">
      <div>
        <h3>Table</h3>
        <p id="matchStatus"></p>
        <div id="tableSeats" class="seat-map"></div>
        <div id="players"></div>
      </div>
      <div>
        <h3>Round Points</h3>
        <div id="roundPoints"></div>
      </div>
      <div>
        <h3>Match Points</h3>
        <div id="scores"></div>
      </div>
      <div>
        <h3>Current Trick</h3>
        <div id="trick"></div>
      </div>
      <div>
        <h3>Last Trick</h3>
        <div id="lastTrick"></div>
      </div>
      <div>
        <h3>Bids</h3>
        <div id="bids"></div>
      </div>
    </div>
    <h3>Round Results</h3>
    <div id="roundResults" class="muted"></div>
    <h3>Actions</h3>
    <div id="bidControls">
      <button onclick="bid(0)">Accept / Pass</button>
      <button onclick="bid(1)">Bid 1</button>
      <button onclick="bid(2)">Bid 2</button>
      <button onclick="bid(3)">Bid 3</button>
      <button onclick="bid(4)">Bid 4</button>
    </div>
    <div id="trumpControls">
      <button onclick="chooseTrump('Hearts')">Trump Hearts</button>
      <button onclick="chooseTrump('Diamonds')">Trump Diamonds</button>
      <button onclick="chooseTrump('Clubs')">Trump Clubs</button>
      <button onclick="chooseTrump('Spades')">Trump Spades</button>
    </div>
    <h3>Your Cards</h3>
    <div id="hand" class="cards"></div>
  </section>
</main>
<script>
let currentUser = localStorage.getItem('cruceUser') || '';
const auth = document.getElementById('auth');
const lobby = document.getElementById('lobby');
const invites = document.getElementById('invites');
const waitingPanel = document.getElementById('waiting');
const rematchPanel = document.getElementById('rematch');
const targetPanel = document.getElementById('target');
const matchPanel = document.getElementById('match');
const authMessage = document.getElementById('authMessage');
const online = document.getElementById('online');
const incoming = document.getElementById('incoming');
const session = document.getElementById('session');
const profile = document.getElementById('profile');
const gameSize = document.getElementById('gameSize');
const gameSizeLabel = document.getElementById('gameSizeLabel');
const waitingStatus = document.getElementById('waitingStatus');
const waitingPlayers = document.getElementById('waitingPlayers');
const waitingChat = document.getElementById('waitingChat');
const chatInput = document.getElementById('chatInput');
const rematchNotice = document.getElementById('rematchNotice');
const rematchResponses = document.getElementById('rematchResponses');
const rematchYes = document.getElementById('rematchYes');
const rematchNo = document.getElementById('rematchNo');
const targetNotice = document.getElementById('targetNotice');
const targetControls = document.getElementById('targetControls');
const turnNotice = document.getElementById('turnNotice');
const matchMessage = document.getElementById('matchMessage');
const matchStatus = document.getElementById('matchStatus');
const tableSeats = document.getElementById('tableSeats');
const players = document.getElementById('players');
const scores = document.getElementById('scores');
const roundPoints = document.getElementById('roundPoints');
const trick = document.getElementById('trick');
const lastTrick = document.getElementById('lastTrick');
const bids = document.getElementById('bids');
const roundResults = document.getElementById('roundResults');
const hand = document.getElementById('hand');
const bidControls = document.getElementById('bidControls');
const trumpControls = document.getElementById('trumpControls');
let currentWaitingRoom = null;
let audioContext = null;
let lastTurnKey = '';
let lastTrickKey = '';
let lastInviteCount = 0;
let lastWinnerKey = '';

function value(id) { return document.getElementById(id).value.trim(); }
function enc(text) { return encodeURIComponent(text); }
function ownerLabel(owner) {
  const match = /^team-(\d+)$/.exec(owner || '');
  return match ? `Team ${Number(match[1]) + 1}` : owner;
}
function teamText(player, state) {
  return state.players.length === 4 && player.team ? ` - ${player.team}` : '';
}

function avatarHtml(initial, color) {
  return `<span class="avatar" style="background:${color || '#153c35'}">${initial || '?'}</span>`;
}

function playTone(kind) {
  try {
    audioContext = audioContext || new (window.AudioContext || window.webkitAudioContext)();
    const osc = audioContext.createOscillator();
    const gain = audioContext.createGain();
    const now = audioContext.currentTime;
    const frequency = kind === 'turn' ? 660 : kind === 'win' ? 880 : kind === 'invite' ? 520 : 440;
    osc.frequency.value = frequency;
    osc.type = 'sine';
    gain.gain.setValueAtTime(0.0001, now);
    gain.gain.exponentialRampToValueAtTime(0.07, now + 0.02);
    gain.gain.exponentialRampToValueAtTime(0.0001, now + 0.16);
    osc.connect(gain).connect(audioContext.destination);
    osc.start(now);
    osc.stop(now + 0.18);
  } catch (_error) {
  }
}

function seatClassFor(seat, count) {
  if (count === 2) return seat === 0 ? 'seat-0' : 'seat-2';
  if (count === 3) return ['seat-0', 'seat-1', 'seat-3'][seat] || 'seat-0';
  return `seat-${seat}`;
}

function renderProfile(profileData) {
  if (!profileData) {
    profile.textContent = 'No profile data yet.';
    return;
  }
  const recent = profileData.recentMatches || [];
  const latest = recent.slice(0, 3).map(match =>
    `${match.completedAt}: ${match.winner} won to ${match.targetScore}`
  );
  profile.innerHTML =
    `${avatarHtml(profileData.avatarInitial, profileData.avatarColor)}` +
    `<div><strong>${profileData.displayName}</strong><br><span class="muted">${profileData.totalWins} total win${profileData.totalWins === 1 ? '' : 's'}</span></div>` +
    `<div class="profile-history">${latest.length ? latest.join('<br>') : 'No completed matches yet.'}</div>`;
}

async function api(path) {
  const response = await fetch(path);
  return await response.json();
}

async function reconnectSavedUser() {
  if (!currentUser) return;
  auth.hidden = true;
  lobby.hidden = false;
  invites.hidden = false;
  const data = await api(`/api/reconnect?username=${enc(currentUser)}&platform=Web`);
  if (!data.ok) {
    localStorage.removeItem('cruceUser');
    currentUser = '';
    auth.hidden = false;
    lobby.hidden = true;
    invites.hidden = true;
    authMessage.textContent = data.message || 'Please log in again.';
    authMessage.className = 'error';
    return;
  }
  refreshLobby();
}

async function login() {
  const data = await api(`/api/login?username=${enc(value('username'))}&password=${enc(value('password'))}&platform=Web`);
  handleAuth(data);
}

async function registerUser() {
  const data = await api(`/api/register?username=${enc(value('username'))}&password=${enc(value('password'))}&platform=Web`);
  handleAuth(data);
}

function handleAuth(data) {
  authMessage.textContent = data.message;
  authMessage.className = data.ok ? 'ok' : 'error';
  if (data.ok) {
    currentUser = data.username;
    localStorage.setItem('cruceUser', currentUser);
    auth.hidden = true;
    lobby.hidden = false;
    invites.hidden = false;
    refreshLobby();
  }
}

async function invite(to) {
  const count = currentWaitingRoom ? currentWaitingRoom.playerCount : Number(gameSize.value || 2);
  let path = `/api/invite?from=${enc(currentUser)}&to=${enc(to)}&players=${count}`;
  if (currentWaitingRoom) {
    path += `&room=${currentWaitingRoom.id}`;
  }
  const data = await api(path);
  alert(data.message);
  refreshLobby();
}

async function respond(inviteId, accept) {
  const data = await api(`/api/respond?username=${enc(currentUser)}&invite=${inviteId}&accept=${accept ? 1 : 0}`);
  alert(data.message);
  refreshLobby();
}

async function selectTarget(score) {
  const data = await api(`/api/target?username=${enc(currentUser)}&score=${score}`);
  matchMessage.textContent = data.message;
  matchMessage.className = data.ok ? 'ok' : 'error';
  await refreshLobby();
}

async function sendChat() {
  const text = chatInput.value.trim();
  if (!text) return;
  const data = await api(`/api/chat?username=${enc(currentUser)}&message=${enc(text)}`);
  session.textContent = data.message;
  session.className = data.ok ? 'ok' : 'error';
  if (data.ok) chatInput.value = '';
  await refreshLobby();
}

chatInput.addEventListener('keydown', event => {
  if (event.key === 'Enter') {
    event.preventDefault();
    sendChat();
  }
});

async function respondRematch(accept) {
  const data = await api(`/api/rematch?username=${enc(currentUser)}&accept=${accept ? 1 : 0}`);
  session.textContent = data.message;
  session.className = data.ok ? 'ok' : 'error';
  await refreshLobby();
}

async function returnToLobby() {
  const data = await api(`/api/cancel?username=${enc(currentUser)}`);
  session.textContent = data.message;
  session.className = data.ok ? 'ok' : 'error';
  matchMessage.textContent = data.message;
  matchMessage.className = data.ok ? 'ok' : 'error';
  await refreshLobby();
}

async function refreshMatch() {
  if (!currentUser) return;
  if (matchPanel.hidden) return;
  const data = await api(`/api/match?username=${enc(currentUser)}`);
  if (!data.ok) {
    matchMessage.textContent = data.message || 'Match is not available.';
    matchPanel.hidden = true;
    lobby.hidden = false;
    invites.hidden = false;
    await refreshLobby();
    return;
  }
  renderMatch(data.state);
}

async function bid(value) {
  const data = await api(`/api/bid?username=${enc(currentUser)}&value=${value}`);
  matchMessage.textContent = data.message;
  matchMessage.className = data.ok ? 'ok' : 'error';
  if (data.state) renderMatch(data.state);
  if (data.ok) await refreshMatch();
}

async function chooseTrump(suit) {
  const data = await api(`/api/trump?username=${enc(currentUser)}&suit=${suit}`);
  matchMessage.textContent = data.message;
  matchMessage.className = data.ok ? 'ok' : 'error';
  if (data.state) renderMatch(data.state);
  if (data.ok) await refreshMatch();
}

async function playCard(cardId) {
  const data = await api(`/api/play?username=${enc(currentUser)}&card=${cardId}`);
  matchMessage.textContent = data.message;
  matchMessage.className = data.ok ? 'ok' : 'error';
  if (data.state) {
    renderMatch(data.state);
    if (data.state.status === 'Complete') return;
  }
  if (data.ok) await refreshMatch();
}

async function refreshLobby() {
  if (!currentUser) return;
  const data = await api(`/api/lobby?username=${enc(currentUser)}`);
  if (!data.ok) {
    auth.hidden = false;
    lobby.hidden = true;
    invites.hidden = true;
    waitingPanel.hidden = true;
    rematchPanel.hidden = true;
    targetPanel.hidden = true;
    matchPanel.hidden = true;
    return;
  }

  currentWaitingRoom = data.waitingRoom || null;
  session.textContent = data.notice || `Logged in as ${currentUser}.`;
  session.className = data.notice ? 'ok' : 'muted';
  gameSizeLabel.hidden = !!currentWaitingRoom;
  renderProfile(data.profile);
  online.innerHTML = '';
  for (const player of data.onlinePlayers) {
    const row = document.createElement('div');
    row.className = 'row';
    const label = document.createElement('span');
    label.className = 'profile-card';
    label.innerHTML =
      `${avatarHtml(player.avatarInitial, player.avatarColor)}` +
      `<span><strong>${player.displayName || player.username}${player.username === currentUser ? ' (you)' : ''}</strong><br>` +
      `<span class="muted">${player.platform} - ${player.inMatch ? 'busy' : 'available'} - ${player.totalWins || 0} win${player.totalWins === 1 ? '' : 's'}</span></span>`;
    row.appendChild(label);
    const canInvite =
      player.username !== currentUser &&
      !player.inMatch &&
      !data.inMatch &&
      !data.targetSelection &&
      !data.rematch;
    if (canInvite) {
      const button = document.createElement('button');
      button.textContent = currentWaitingRoom ? 'Invite to Room' : 'Invite';
      button.onclick = () => invite(player.username);
      row.appendChild(button);
    }
    online.appendChild(row);
  }

  incoming.innerHTML = '';
  incoming.className = '';
  const inviteCount = data.invitations.length;
  if (inviteCount > lastInviteCount) playTone('invite');
  lastInviteCount = inviteCount;
  for (const inv of data.invitations) {
    const row = document.createElement('div');
    row.className = 'row';
    row.innerHTML = `<span>${inv.from} invited you to a ${inv.playerCount}-player game.</span>`;
    const controls = document.createElement('span');
    const accept = document.createElement('button');
    accept.textContent = 'Accept';
    accept.onclick = () => respond(inv.id, true);
    const decline = document.createElement('button');
    decline.textContent = 'Decline';
    decline.onclick = () => respond(inv.id, false);
    controls.appendChild(accept);
    controls.appendChild(decline);
    row.appendChild(controls);
    incoming.appendChild(row);
  }
  if (data.invitations.length === 0) {
    incoming.textContent = 'No pending invitations.';
    incoming.className = 'muted';
  }

  if (data.inMatch) {
    lobby.hidden = true;
    invites.hidden = true;
    waitingPanel.hidden = true;
    rematchPanel.hidden = true;
    targetPanel.hidden = true;
    matchPanel.hidden = false;
    refreshMatch();
  } else if (data.targetSelection) {
    lobby.hidden = false;
    invites.hidden = true;
    waitingPanel.hidden = true;
    rematchPanel.hidden = true;
    matchPanel.hidden = true;
    targetPanel.hidden = false;
    renderTargetSelection(data.targetSelection);
  } else if (data.rematch) {
    lobby.hidden = true;
    invites.hidden = true;
    waitingPanel.hidden = true;
    rematchPanel.hidden = false;
    targetPanel.hidden = true;
    matchPanel.hidden = true;
    renderRematch(data.rematch);
  } else if (currentWaitingRoom) {
    lobby.hidden = false;
    invites.hidden = true;
    waitingPanel.hidden = false;
    rematchPanel.hidden = true;
    targetPanel.hidden = true;
    matchPanel.hidden = true;
    renderWaitingRoom(currentWaitingRoom);
  } else {
    lobby.hidden = false;
    invites.hidden = false;
    waitingPanel.hidden = true;
    rematchPanel.hidden = true;
    targetPanel.hidden = true;
    matchPanel.hidden = true;
  }
}

function renderRematch(rematch) {
  const responded = !!rematch.responded;
  const accepted = Object.keys(rematch.responses || {}).length;
  rematchNotice.textContent = responded
    ? `Waiting for the other player(s). ${accepted}/${rematch.players.length} accepted.`
    : `${rematch.winner} won the match. Play a rematch?`;
  rematchNotice.className = responded ? 'turn-banner waiting' : 'turn-banner ready';
  rematchResponses.textContent = rematch.players
    .map(player => `${player}: ${rematch.responses && rematch.responses[player] ? 'yes' : 'waiting'}`)
    .join(' | ');
  rematchYes.disabled = responded;
  rematchNo.disabled = responded;
}

function renderWaitingRoom(room) {
  const remaining = room.playerCount - room.players.length;
  waitingStatus.textContent = remaining > 0
    ? `Waiting for ${remaining} more player${remaining === 1 ? '' : 's'}.`
    : 'Room is full. Waiting for target score selection.';
  waitingStatus.className = 'turn-banner waiting';
  waitingPlayers.innerHTML = '';
  for (const player of room.players) {
    const div = document.createElement('div');
    div.textContent = `${player}${player === currentUser ? ' - you' : ''}`;
    waitingPlayers.appendChild(div);
  }

  waitingChat.innerHTML = '';
  for (const item of room.messages || []) {
    const div = document.createElement('div');
    div.className = 'chat-line';
    div.textContent = `${item.from}: ${item.message}`;
    waitingChat.appendChild(div);
  }
  if ((room.messages || []).length === 0) {
    waitingChat.textContent = 'No messages yet.';
  }
}

function renderTargetSelection(selection) {
  const isChooser = selection.chooser === currentUser;
  targetNotice.textContent = isChooser
    ? `It is your turn: select the target score for ${selection.playerCount} players.`
    : `Waiting for ${selection.chooser} to select the target score for ${selection.playerCount} players.`;
  targetNotice.className = isChooser ? 'turn-banner ready' : 'turn-banner waiting';
  for (const button of targetControls.querySelectorAll('button')) {
    button.disabled = !isChooser;
  }
}

function renderSeatMap(state, isMyTurn) {
  tableSeats.innerHTML = '';
  const count = (state.players || []).length;
  for (const player of state.players || []) {
    const div = document.createElement('div');
    div.className = `seat ${seatClassFor(player.seat, count)}${state.currentTurn === player.id ? ' active' : ''}`;
    const initial = (player.name || player.id || '?').charAt(0).toUpperCase();
    div.innerHTML =
      `${avatarHtml(initial, player.id === currentUser ? '#145331' : '#153c35')}` +
      `<span><strong>${player.name}${player.id === currentUser ? ' (you)' : ''}</strong><br>` +
      `<span class="muted">Seat ${player.seat + 1} - ${player.cardsInHand} cards${teamText(player, state)}${player.connected ? '' : ' - disconnected'}</span></span>`;
    tableSeats.appendChild(div);
  }
  const center = document.createElement('div');
  center.className = 'seat table-center';
  center.innerHTML = `<span><strong>${isMyTurn ? 'Your move' : 'Table'}</strong><br><span class="muted">${state.status}</span></span>`;
  tableSeats.appendChild(center);
}

function renderMatch(state) {
  const isMyTurn = state.currentTurn === currentUser;
  const turnKey = `${state.matchId}:${state.status}:${state.currentTurn || ''}`;
  if (turnKey !== lastTurnKey && isMyTurn && state.status !== 'Complete') playTone('turn');
  lastTurnKey = turnKey;
  const trickKey = JSON.stringify((state.currentTrick || []).map(played => `${played.player}:${played.card && played.card.id}`));
  if (trickKey !== lastTrickKey && trickKey !== '[]') playTone('card');
  lastTrickKey = trickKey;
  const winnerKey = `${state.matchId}:${state.winner || ''}`;
  if (state.status === 'Complete' && state.winner && winnerKey !== lastWinnerKey) playTone('win');
  lastWinnerKey = winnerKey;

  if (state.status === 'Complete') {
    turnNotice.textContent = `${state.winner || 'A player'} won the match. Rematch prompt will appear.`;
    turnNotice.className = 'turn-banner ready';
    bidControls.hidden = true;
    trumpControls.hidden = true;
    setTimeout(refreshLobby, 1500);
  } else {
    const actionText = state.status === 'Bidding'
      ? 'select a bid or accept/pass'
      : state.status === 'ChoosingTrump'
        ? 'choose trump'
        : state.status === 'Playing'
          ? 'play a legal card'
          : 'wait for the next round';
    turnNotice.textContent = isMyTurn
      ? `It's your turn: ${actionText}.`
      : `Waiting for ${state.currentTurn || 'the server'}.`;
    turnNotice.className = isMyTurn ? 'turn-banner ready' : 'turn-banner waiting';
  }

  matchStatus.innerHTML =
    `<span class="pill">Match ${state.matchId}</span>` +
    `<span class="pill">${state.status}</span>` +
    `<span class="pill">Turn: ${state.currentTurn || 'none'}</span>` +
    `<span class="pill">Trump: ${state.trump || 'not chosen'}</span>` +
    `<span class="pill">Target: ${state.targetScore}</span>`;

  players.innerHTML = '';
  players.textContent = 'Only your own cards are visible. Other players show card counts only.';
  players.className = 'muted';
  renderSeatMap(state, isMyTurn);

  scores.innerHTML = '';
  for (const [owner, score] of Object.entries(state.scores || {})) {
    const div = document.createElement('div');
    div.textContent = `${ownerLabel(owner)}: ${score}`;
    scores.appendChild(div);
  }

  roundPoints.innerHTML = '';
  for (const [owner, points] of Object.entries(state.roundPoints || {})) {
    const div = document.createElement('div');
    div.textContent = `${ownerLabel(owner)}: ${points}`;
    roundPoints.appendChild(div);
  }
  if (Object.keys(state.roundPoints || {}).length === 0) {
    roundPoints.textContent = 'No round points yet.';
  }

  trick.innerHTML = '';
  for (const played of state.currentTrick || []) {
    trick.appendChild(playedCardElement(played));
  }
  if ((state.currentTrick || []).length === 0) trick.textContent = 'No cards on table.';

  lastTrick.innerHTML = '';
  for (const played of state.lastCompletedTrick || []) {
    lastTrick.appendChild(playedCardElement(played));
  }
  if ((state.lastCompletedTrick || []).length === 0) {
    lastTrick.textContent = 'No completed tricks yet.';
  } else if (state.lastTrickWinner) {
    const winner = document.createElement('div');
    winner.className = 'ok';
    winner.textContent = `${state.lastTrickWinner} won and leads next.`;
    lastTrick.appendChild(winner);
  }

  bids.innerHTML = '';
  for (const bid of state.bids || []) {
    const div = document.createElement('div');
    div.textContent = `${bid.player}: ${bid.passed ? 'accepted / passed' : `bid ${bid.value}`}`;
    bids.appendChild(div);
  }
  if ((state.bids || []).length === 0) bids.textContent = 'No bids yet.';

  roundResults.innerHTML = '';
  roundResults.className = (state.roundResults || []).length === 0 ? 'muted' : '';
  if ((state.roundResults || []).length === 0) {
    roundResults.textContent = 'No completed rounds yet.';
  } else {
    for (const result of state.roundResults || []) {
      const div = document.createElement('div');
      const deltas = Object.entries(result.matchPointDelta || {})
        .map(([owner, delta]) => `${ownerLabel(owner)} ${delta >= 0 ? '+' : ''}${delta}`)
        .join(', ');
      div.textContent = `Round ${result.roundNumber}: ${deltas}`;
      roundResults.appendChild(div);
    }
  }

  bidControls.hidden = !(state.status === 'Bidding' && isMyTurn);
  trumpControls.hidden = !(state.status === 'ChoosingTrump' && isMyTurn);

  hand.innerHTML = '';
  const canPlay = state.status === 'Playing' && isMyTurn;
  const legalCards = new Set((state.legalCardIds || []).map(Number));
  if (canPlay && legalCards.size < (state.ownHand || []).length) {
    const hint = document.createElement('div');
    hint.className = 'muted';
    hint.style.flexBasis = '100%';
    hint.textContent = 'Only highlighted cards are legal for this trick.';
    hand.appendChild(hint);
  }
  for (const card of state.ownHand || []) {
    const legal = canPlay && legalCards.has(Number(card.id));
    const button = document.createElement('button');
    button.className = `card-button ${legal ? 'legal-card' : 'illegal-card'}`;
    button.disabled = !legal;
    button.title = legal ? 'Play this card' : 'This card is not legal to play right now.';
    button.onclick = () => playCard(card.id);
    const image = document.createElement('img');
    image.src = card.image;
    image.alt = card.label;
    const label = document.createElement('span');
    label.textContent = card.label;
    button.appendChild(image);
    button.appendChild(label);
    hand.appendChild(button);
  }
}

function playedCardElement(played) {
  const row = document.createElement('div');
  row.className = 'played-card';
  const image = document.createElement('img');
  image.src = played.card.image;
  image.alt = played.card.label;
  const label = document.createElement('span');
  label.textContent = `${played.player}: ${played.card.label}`;
  row.appendChild(image);
  row.appendChild(label);
  return row;
}

reconnectSavedUser();
setInterval(refreshLobby, 2000);
setInterval(refreshMatch, 2000);
</script>
</body>
</html>)HTML";
}

class GameDataStore {
 public:
  GameDataStore(std::filesystem::path events_path, std::filesystem::path history_path)
      : events_path_(std::move(events_path)), history_path_(std::move(history_path)) {
    load_history();
  }

  PlayerProfile profile_for(const std::string& username) const {
    PlayerProfile profile;
    profile.username = username;
    profile.display_name = username;
    profile.avatar_initial =
        username.empty()
            ? "?"
            : std::string(
                  1,
                  static_cast<char>(
                      std::toupper(static_cast<unsigned char>(username.front()))));
    profile.avatar_color = avatar_color(username);
    if (const auto found = wins_by_player_.find(username); found != wins_by_player_.end()) {
      profile.total_wins = found->second;
    }
    if (const auto found = history_by_player_.find(username); found != history_by_player_.end()) {
      profile.recent_matches = found->second;
    }
    return profile;
  }

  void log_action(
      const std::string& action,
      const std::string& actor,
      const std::string& request_json,
      bool ok,
      const std::string& message,
      const std::optional<cruce::server::GameSnapshot>& before,
      const std::optional<cruce::server::GameSnapshot>& after) const {
    ensure_parent(events_path_);
    std::ofstream output(events_path_, std::ios::app);
    if (!output) {
      return;
    }

    std::string match_id;
    if (after.has_value()) {
      match_id = after->match_id;
    } else if (before.has_value()) {
      match_id = before->match_id;
    }

    output << "{\"type\":\"move\""
           << ",\"timestamp\":" << json_string(utc_timestamp())
           << ",\"actor\":" << json_string(actor)
           << ",\"action\":" << json_string(action)
           << ",\"matchId\":" << json_string(match_id)
           << ",\"ok\":" << (ok ? "true" : "false")
           << ",\"message\":" << json_string(message)
           << ",\"request\":" << request_json;
    if (before.has_value()) {
      output << ",\"stateBefore\":" << snapshot_json(*before);
    }
    if (after.has_value()) {
      output << ",\"stateAfter\":" << snapshot_json(*after);
    }
    output << "}\n";
  }

  void record_completed_match(const cruce::server::GameSnapshot& state) {
    if (completed_match_ids_.contains(state.match_id)) {
      return;
    }
    completed_match_ids_.insert(state.match_id);

    MatchHistoryEntry entry;
    entry.match_id = state.match_id;
    entry.completed_at = utc_timestamp();
    entry.winner = state.winner.value_or("");
    entry.target_score = state.target_score;
    entry.final_score = state.scores;
    entry.rounds = static_cast<int>(state.round_results.size());
    for (const auto& player : state.players) {
      entry.players.push_back(player.player_id);
      if (player.owner_id == entry.winner || player.player_id == entry.winner) {
        entry.winning_players.push_back(player.player_id);
      }
    }

    append_history(entry);
    remember_history(entry);
  }

 private:
  static void ensure_parent(const std::filesystem::path& path) {
    const auto parent = path.parent_path();
    if (!parent.empty()) {
      std::filesystem::create_directories(parent);
    }
  }

  static std::string avatar_color(const std::string& username) {
    static const std::vector<std::string> colors{
        "#153c35", "#a84f2b", "#2f6f68", "#6d5a1e", "#7a3d57", "#2f4f74"};
    std::size_t hash = 0;
    for (const unsigned char ch : username) {
      hash = (hash * 131u) + ch;
    }
    return colors[hash % colors.size()];
  }

  static std::string score_text(const std::map<std::string, int>& scores) {
    std::ostringstream out;
    bool first = true;
    for (const auto& [owner, score] : scores) {
      if (!first) {
        out << ",";
      }
      out << owner << "=" << score;
      first = false;
    }
    return out.str();
  }

  static std::map<std::string, int> parse_score_text(const std::string& text) {
    std::map<std::string, int> scores;
    for (const auto& item : split_text(text, ',')) {
      const auto mark = item.find('=');
      if (mark == std::string::npos) {
        continue;
      }
      try {
        scores[item.substr(0, mark)] = std::stoi(item.substr(mark + 1));
      } catch (const std::exception&) {
      }
    }
    return scores;
  }

  void append_history(const MatchHistoryEntry& entry) const {
    ensure_parent(history_path_);
    std::ofstream output(history_path_, std::ios::app);
    if (!output) {
      return;
    }
    output << entry.match_id << "\t"
           << entry.completed_at << "\t"
           << entry.winner << "\t"
           << entry.target_score << "\t"
           << score_text(entry.final_score) << "\t"
           << join_text(entry.players, ",") << "\t"
           << entry.rounds << "\t"
           << join_text(entry.winning_players, ",") << "\n";
  }

  void load_history() {
    if (!std::filesystem::exists(history_path_)) {
      return;
    }
    std::ifstream input(history_path_);
    if (!input) {
      return;
    }

    std::string line;
    while (std::getline(input, line)) {
      if (line.empty() || line.starts_with("#")) {
        continue;
      }
      const auto fields = split_with_empty_fields(line, '\t');
      if (fields.size() < 7) {
        continue;
      }

      MatchHistoryEntry entry;
      entry.match_id = fields[0];
      entry.completed_at = fields[1];
      entry.winner = fields[2];
      try {
        entry.target_score = std::stoi(fields[3]);
      } catch (const std::exception&) {
        continue;
      }
      entry.final_score = parse_score_text(fields[4]);
      entry.players = split_text(fields[5], ',');
      try {
        entry.rounds = std::stoi(fields[6]);
      } catch (const std::exception&) {
        entry.rounds = 0;
      }
      if (fields.size() >= 8) {
        entry.winning_players = split_text(fields[7], ',');
      } else if (!is_team_owner(entry.winner)) {
        entry.winning_players.push_back(entry.winner);
      }
      remember_history(entry);
    }
  }

  static std::vector<std::string> split_with_empty_fields(
      const std::string& text,
      char delimiter) {
    std::vector<std::string> values;
    std::string item;
    std::istringstream input(text);
    while (std::getline(input, item, delimiter)) {
      values.push_back(item);
    }
    return values;
  }

  void remember_history(const MatchHistoryEntry& entry) {
    for (const auto& player : entry.players) {
      auto& history = history_by_player_[player];
      history.insert(history.begin(), entry);
      if (history.size() > 8) {
        history.resize(8);
      }
      if (contains_text(entry.winning_players, player)) {
        ++wins_by_player_[player];
      }
    }
  }

  std::filesystem::path events_path_;
  std::filesystem::path history_path_;
  std::set<std::string> completed_match_ids_;
  std::map<std::string, int> wins_by_player_;
  std::map<std::string, std::vector<MatchHistoryEntry>> history_by_player_;
};

struct SelfPlayOptions {
  int games = 500;
  int player_count = 0;
  int target_score = 6;
  std::uint32_t seed = 20260813;
  std::filesystem::path events_path = "data/game_events.db";
  std::filesystem::path history_path = "data/match_history.db";
};

struct SelfPlayStats {
  int requested = 0;
  int completed = 0;
  int failed = 0;
  int actions = 0;
};

class SelfPlaySimulator {
 public:
  SelfPlaySimulator(GameDataStore& data_store, std::uint32_t seed)
      : data_store_(data_store), rng_(seed), run_id_(make_run_id(seed)) {}

  // Runs a batch of complete bot-vs-bot games and records every action so the
  // Python training scripts can later learn from those decisions.
  SelfPlayStats run(const SelfPlayOptions& options) {
    SelfPlayStats stats;
    stats.requested = options.games;
    for (int game_index = 1; game_index <= options.games; ++game_index) {
      if (run_one_game(game_index, options, stats)) {
        ++stats.completed;
      } else {
        ++stats.failed;
      }

      if (game_index % 50 == 0 || game_index == options.games) {
        std::cout << "Self-play progress: " << game_index << "/" << options.games
                  << " games, " << stats.completed << " completed, "
                  << stats.failed << " failed.\n";
      }
    }
    return stats;
  }

 private:
  // Build a stable prefix for generated bot names so rows from one self-play
  // run can be grouped together in the event log.
  static std::string make_run_id(std::uint32_t seed) {
    std::string timestamp = utc_timestamp();
    for (auto& ch : timestamp) {
      if (ch == ':' || ch == '-' || ch == 'T' || ch == 'Z') {
        ch = '_';
      }
    }
    return "sp_" + std::to_string(seed) + "_" + timestamp;
  }

  // If the command line did not specify a mode, choose 2, 3, or 4 players.
  int choose_player_count(const SelfPlayOptions& options) {
    if (options.player_count >= 2 && options.player_count <= 4) {
      return options.player_count;
    }
    return std::uniform_int_distribution<int>(2, 4)(rng_);
  }

  // Target score stays within the Cruce-supported values: 6, 11, or 21.
  int choose_target_score(const SelfPlayOptions& options) {
    if (cruce::is_valid_target_score(options.target_score)) {
      return options.target_score;
    }
    return 6;
  }

  // Create unique fake usernames for the bot seats in one generated game.
  std::vector<std::string> bot_players(int game_index, int player_count) const {
    std::vector<std::string> players;
    for (int seat = 0; seat < player_count; ++seat) {
      players.push_back(
          run_id_ + "_g" + std::to_string(game_index) + "_p" + std::to_string(seat));
    }
    return players;
  }

  // Drives one full match through the same GameServer API used by real clients,
  // which means self-play data follows the same validation rules as live games.
  bool run_one_game(int game_index, const SelfPlayOptions& options, SelfPlayStats& stats) {
    const int player_count = choose_player_count(options);
    const int target_score = choose_target_score(options);
    const auto players = bot_players(game_index, player_count);
    const auto room_code = server_.create_room(player_count, target_score);

    // Seat all generated bots in the room. The room automatically starts once
    // the requested player count has joined.
    for (const auto& player : players) {
      const auto joined = server_.join_room(
          room_code,
          cruce::server::JoinRequest{player, player, cruce::ClientPlatform::Bot});
      if (!joined) {
        std::cerr << "Self-play join failed: " << joined.message << "\n";
        return false;
      }
    }

    // The first snapshot confirms that the match exists and provides a complete
    // private state for one player.
    const auto initial_state = server_.snapshot_for(players.front());
    if (!initial_state.has_value()) {
      std::cerr << "Self-play could not start a match.\n";
      return false;
    }

    // A start event marks the game boundaries in data/game_events.db.
    data_store_.log_action(
        "self_play_start",
        "self-play",
        "{\"gameIndex\":" + std::to_string(game_index) +
            ",\"playerCount\":" + std::to_string(player_count) +
            ",\"targetScore\":" + std::to_string(target_score) +
            ",\"players\":" + string_array_json(players) + "}",
        true,
        "Self-play match started.",
        std::nullopt,
        initial_state);
    ++stats.actions;

    const int action_limit = 2500 + target_score * 500;
    for (int action_index = 0; action_index < action_limit; ++action_index) {
      // Use any player's snapshot to inspect public table state, then switch to
      // the current actor's private snapshot before choosing the action.
      const auto table_state = server_.snapshot_for(players.front());
      if (!table_state.has_value()) {
        std::cerr << "Self-play lost the active match state.\n";
        return false;
      }

      // Completed matches are persisted and then removed from active memory.
      if (table_state->status == cruce::MatchStatus::Complete) {
        complete_game(*table_state);
        return true;
      }

      if (!table_state->current_turn_player.has_value()) {
        std::cerr << "Self-play found no current turn player.\n";
        return false;
      }

      const auto actor = *table_state->current_turn_player;
      auto before = server_.snapshot_for(actor);
      if (!before.has_value()) {
        std::cerr << "Self-play actor has no private state.\n";
        return false;
      }

      // Bidding, trump choice, and card play are logged with before/after state
      // so the exporter can create supervised learning examples.
      if (before->status == cruce::MatchStatus::Bidding) {
        const int bid_value = choose_bid(*before);
        const auto result = server_.submit_bid(actor, bid_value);
        const auto after = server_.snapshot_for(actor);
        data_store_.log_action(
            "self_play_bid",
            actor,
            "{\"value\":" + std::to_string(bid_value) + "}",
            result.ok,
            result.message,
            before,
            after);
        ++stats.actions;
        if (!result) {
          std::cerr << "Self-play bid failed: " << result.message << "\n";
          return false;
        }
      } else if (before->status == cruce::MatchStatus::ChoosingTrump) {
        const auto suit = choose_trump(*before);
        const auto result = server_.choose_trump(actor, suit);
        const auto after = server_.snapshot_for(actor);
        data_store_.log_action(
            "self_play_choose_trump",
            actor,
            "{\"suit\":" + json_string(cruce::to_string(suit)) + "}",
            result.ok,
            result.message,
            before,
            after);
        ++stats.actions;
        if (!result) {
          std::cerr << "Self-play trump choice failed: " << result.message << "\n";
          return false;
        }
      } else if (before->status == cruce::MatchStatus::Playing) {
        const auto card = choose_card(*before);
        if (!card.has_value()) {
          std::cerr << "Self-play found no legal card.\n";
          return false;
        }
        const auto result = server_.submit_card_play(actor, *card);
        const auto after = server_.snapshot_for(actor);
        data_store_.log_action(
            "self_play_play_card",
            actor,
            "{\"cardId\":" + std::to_string(card->id()) +
                ",\"card\":" + card_json(*card) + "}",
            result.ok,
            result.message,
            before,
            after);
        ++stats.actions;
        if (!result) {
          std::cerr << "Self-play card play failed: " << result.message << "\n";
          return false;
        }
        if (after.has_value() && after->status == cruce::MatchStatus::Complete) {
          complete_game(*after);
          return true;
        }
      } else {
        std::cerr << "Self-play reached unsupported match state.\n";
        return false;
      }
    }

    std::cerr << "Self-play exceeded the action limit.\n";
    return false;
  }

  // Records the completed match summary and frees the server's active match slot.
  void complete_game(const cruce::server::GameSnapshot& state) {
    data_store_.record_completed_match(state);
    server_.release_completed_match(state.match_id);
  }

  // Delegate decisions to the same heuristic used by the live AI players.
  int choose_bid(const cruce::server::GameSnapshot& state) {
    return CruceHeuristicBot::choose_bid(state);
  }

  cruce::Suit choose_trump(const cruce::server::GameSnapshot& state) {
    return CruceHeuristicBot::choose_trump(state);
  }

  std::optional<cruce::Card> choose_card(const cruce::server::GameSnapshot& state) {
    return CruceHeuristicBot::choose_card(state);
  }

  GameDataStore& data_store_;
  cruce::server::GameServer server_;
  std::mt19937 rng_;
  std::string run_id_;
};

class LobbyState {
 public:
  explicit LobbyState(std::filesystem::path user_db_path)
      : users_(std::move(user_db_path)),
        data_store_("data/game_events.db", "data/match_history.db") {
    const auto initialized = users_.initialize();
    if (!initialized) {
      std::cerr << initialized.message << "\n";
    }
    initialize_ai_players();
  }

  std::string login(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    const auto password = param(params, "password");
    const auto result = users_.authenticate(username, password);
    if (result) {
      online_users_.insert(username);
      platforms_[username] = parse_platform(param(params, "platform", "Web"));
    }
    return auth_json(result, username);
  }

  std::string register_user(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    const auto password = param(params, "password");
    if (is_ai_player(username)) {
      return simple_json(false, "That username is reserved for an AI player.");
    }
    const auto result = users_.register_user(username, password);
    if (result) {
      online_users_.insert(username);
      platforms_[username] = parse_platform(param(params, "platform", "Web"));
    }
    return auth_json(result, username);
  }

  std::string reconnect(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!users_.exists(username)) {
      return simple_json(false, "Unknown username.");
    }
    online_users_.insert(username);
    platforms_[username] = parse_platform(param(params, "platform", "Web"));
    server_.reconnect_player(username);
    return auth_json(cruce::ValidationResult::success("Session restored."), username);
  }

  std::string lobby_json(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return "{\"ok\":false,\"message\":\"User is not logged in.\"}";
    }
    advance_ai();

    std::ostringstream out;
    out << "{\"ok\":true,\"username\":" << json_string(username);
    const auto match = server_.snapshot_for(username);
    const auto target_selection = target_selection_for(username);
    const auto waiting_room = waiting_room_for(username);
    const auto rematch_offer = rematch_offer_for(username);
    out << ",\"inMatch\":" << (match.has_value() ? "true" : "false");
    out << ",\"matchId\":";
    if (match.has_value()) {
      out << json_string(match->match_id);
    } else {
      out << "null";
    }
    out << ",\"notice\":";
    if (const auto notice = notices_.find(username); notice != notices_.end()) {
      out << json_string(notice->second);
    } else {
      out << "null";
    }
    out << ",\"profile\":" << profile_json(data_store_.profile_for(username));
    out << ",\"targetSelection\":";
    if (target_selection != nullptr) {
      out << "{\"id\":" << target_selection->id
          << ",\"playerCount\":" << target_selection->player_count
          << ",\"chooser\":" << json_string(target_selection->chooser)
          << ",\"players\":" << string_array_json(target_selection->players) << "}";
    } else {
      out << "null";
    }
    out << ",\"waitingRoom\":";
    if (waiting_room != nullptr) {
      out << "{\"id\":" << waiting_room->id
          << ",\"playerCount\":" << waiting_room->player_count
          << ",\"players\":" << string_array_json(waiting_room->players)
          << ",\"messages\":[";
      for (std::size_t index = 0; index < waiting_room->messages.size(); ++index) {
        if (index > 0) {
          out << ",";
        }
        out << "{\"from\":" << json_string(waiting_room->messages[index].from)
            << ",\"message\":" << json_string(waiting_room->messages[index].message) << "}";
      }
      out << "]}";
    } else {
      out << "null";
    }
    out << ",\"rematch\":";
    if (rematch_offer != nullptr) {
      out << "{\"id\":" << rematch_offer->id
          << ",\"playerCount\":" << rematch_offer->player_count
          << ",\"winner\":" << json_string(rematch_offer->winner)
          << ",\"players\":" << string_array_json(rematch_offer->players)
          << ",\"responses\":" << bool_map_json(rematch_offer->accepted_by_player)
          << ",\"responded\":"
          << (rematch_offer->accepted_by_player.contains(username) ? "true" : "false")
          << "}";
    } else {
      out << "null";
    }

    out << ",\"onlinePlayers\":[";
    bool first = true;
    for (const auto& online_user : online_users_) {
      if (!first) {
        out << ",";
      }
      const bool in_match = is_busy(online_user);
      const auto profile = data_store_.profile_for(online_user);
      out << "{\"username\":" << json_string(online_user)
          << ",\"displayName\":" << json_string(profile.display_name)
          << ",\"avatarInitial\":" << json_string(profile.avatar_initial)
          << ",\"avatarColor\":" << json_string(profile.avatar_color)
          << ",\"totalWins\":" << profile.total_wins
          << ",\"platform\":" << json_string(cruce::to_string(platforms_[online_user]))
          << ",\"inMatch\":" << (in_match ? "true" : "false") << "}";
      first = false;
    }
    out << "]";

    out << ",\"invitations\":[";
    first = true;
    for (const auto& invitation : invitations_) {
      if (invitation.to == username && invitation.status == "pending") {
        if (!first) {
          out << ",";
        }
        out << "{\"id\":" << invitation.id
            << ",\"from\":" << json_string(invitation.from)
            << ",\"playerCount\":" << invitation.player_count;
        if (invitation.waiting_room_id.has_value()) {
          out << ",\"waitingRoomId\":" << *invitation.waiting_room_id;
        } else {
          out << ",\"waitingRoomId\":null";
        }
        out << "}";
        first = false;
      }
    }
    out << "]}";
    return out.str();
  }

  std::string lobby_text(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return "ERROR User is not logged in.\n";
    }

    std::ostringstream out;
    out << "USER " << username << "\n";
    out << "NOTICE\n";
    if (const auto notice = notices_.find(username); notice != notices_.end()) {
      out << notice->second << "\n";
    }
    out << "ONLINE\n";
    for (const auto& online_user : online_users_) {
      const bool in_match = is_busy(online_user);
      out << online_user << "|" << cruce::to_string(platforms_[online_user]) << "|"
          << (in_match ? "in_match" : "available") << "\n";
    }

    out << "INVITES\n";
    for (const auto& invitation : invitations_) {
      if (invitation.to == username && invitation.status == "pending") {
        out << invitation.id << "|" << invitation.from << "|"
            << invitation.player_count << "|"
            << (invitation.waiting_room_id.has_value()
                    ? std::to_string(*invitation.waiting_room_id)
                    : "0")
            << "\n";
      }
    }

    out << "MATCH\n";
    if (const auto match = server_.snapshot_for(username)) {
      out << match->match_id << "|" << cruce::to_string(match->status) << "\n";
    }

    out << "TARGET\n";
    if (const auto target_selection = target_selection_for(username)) {
      out << target_selection->id << "|" << target_selection->chooser << "|"
          << target_selection->player_count << "|"
          << join_text(target_selection->players, ",") << "\n";
    }

    out << "WAITING\n";
    if (const auto waiting_room = waiting_room_for(username)) {
      out << waiting_room->id << "|" << waiting_room->player_count << "|"
          << join_text(waiting_room->players, ",") << "\n";
      out << "CHAT\n";
      for (const auto& message : waiting_room->messages) {
        out << message.from << "|" << message.message << "\n";
      }
    }
    return out.str();
  }

  std::string match_json(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return "{\"ok\":false,\"message\":\"User is not logged in.\"}";
    }
    advance_ai();

    const auto state = server_.snapshot_for(username);
    if (!state.has_value()) {
      return "{\"ok\":false,\"message\":\"User is not in a match.\"}";
    }

    return "{\"ok\":true,\"state\":" + snapshot_json(*state) + "}";
  }

  std::string match_text(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return "ERROR User is not logged in.\n";
    }

    const auto state = server_.snapshot_for(username);
    if (!state.has_value()) {
      return "ERROR User is not in a match.\n";
    }

    std::ostringstream out;
    out << "STATUS\n";
    out << state->match_id << "|" << cruce::to_string(state->status) << "|"
        << (state->current_turn_player.has_value() ? *state->current_turn_player : "none") << "|"
        << (state->trump_suit.has_value() ? cruce::to_string(*state->trump_suit) : "none")
        << "|" << state->target_score << "\n";

    out << "PLAYERS\n";
    for (const auto& player : state->players) {
      out << player.player_id << "|" << player.display_name << "|"
          << player.cards_in_hand << "|" << player.owner_id << "\n";
    }

    out << "ROUND_POINTS\n";
    for (const auto& [owner, points] : state->round_points) {
      out << owner << "|" << points << "\n";
    }

    out << "MATCH_POINTS\n";
    for (const auto& [owner, score] : state->scores) {
      out << owner << "|" << score << "\n";
    }

    out << "ROUND_RESULTS\n";
    for (const auto& result : state->round_results) {
      out << result.round_number << "|" << result.bid_winner << "|"
          << result.bid_value << "|" << cruce::to_string(result.trump_suit) << "|"
          << (result.bid_succeeded ? "success" : "failed") << "\n";
      for (const auto& [owner, points] : result.card_points) {
        out << "card|" << owner << "|" << points << "\n";
      }
      for (const auto& [owner, delta] : result.match_point_delta) {
        out << "match|" << owner << "|" << delta << "\n";
      }
    }

    out << "BIDS\n";
    for (const auto& bid : state->bids) {
      out << bid.player_id << "|" << (bid.passed ? "pass" : std::to_string(bid.value)) << "\n";
    }

    out << "TRICK\n";
    for (const auto& played : state->current_trick) {
      out << played.player_id << "|" << played.card.id() << "|" << played.card.label() << "\n";
    }

    out << "LAST_TRICK\n";
    if (state->last_trick_winner.has_value()) {
      out << "winner|" << *state->last_trick_winner << "\n";
    }
    for (const auto& played : state->last_completed_trick) {
      out << played.player_id << "|" << played.card.id() << "|" << played.card.label() << "\n";
    }

    out << "HAND\n";
    for (const auto& card : state->own_hand) {
      out << card.id() << "|" << card.label() << "\n";
    }
    return out.str();
  }

  std::string bid(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return action_json(cruce::ValidationResult::failure("User is not logged in."), std::nullopt);
    }

    const int value = std::stoi(param(params, "value", "0"));
    const auto before = server_.snapshot_for(username);
    const auto result = server_.submit_bid(username, value);
    const auto after = server_.snapshot_for(username);
    data_store_.log_action(
        "bid",
        username,
        "{\"value\":" + std::to_string(value) + "}",
        result.ok,
        result.message,
        before,
        after);
    if (result.ok) {
      advance_ai();
    }
    return action_json(result, server_.snapshot_for(username));
  }

  std::string choose_trump(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return action_json(cruce::ValidationResult::failure("User is not logged in."), std::nullopt);
    }

    const auto suit = parse_suit(param(params, "suit"));
    if (!suit.has_value()) {
      return action_json(
          cruce::ValidationResult::failure("Unknown trump suit."),
          server_.snapshot_for(username));
    }

    const auto before = server_.snapshot_for(username);
    const auto result = server_.choose_trump(username, *suit);
    const auto after = server_.snapshot_for(username);
    data_store_.log_action(
        "choose_trump",
        username,
        "{\"suit\":" + json_string(param(params, "suit")) + "}",
        result.ok,
        result.message,
        before,
        after);
    if (result.ok) {
      advance_ai();
    }
    return action_json(result, server_.snapshot_for(username));
  }

  std::string play_card(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return action_json(cruce::ValidationResult::failure("User is not logged in."), std::nullopt);
    }

    const auto state = server_.snapshot_for(username);
    if (!state.has_value()) {
      return action_json(cruce::ValidationResult::failure("User is not in a match."), std::nullopt);
    }

    const int card_id = std::stoi(param(params, "card", "-1"));
    const auto card = find_own_card(*state, card_id);
    if (!card.has_value()) {
      data_store_.log_action(
          "play_card",
          username,
          "{\"cardId\":" + std::to_string(card_id) + "}",
          false,
          "Card is not in the player's hand.",
          state,
          state);
      return action_json(
          cruce::ValidationResult::failure("Card is not in the player's hand."),
          state);
    }

    const auto result = server_.submit_card_play(username, *card);
    const auto updated_state = server_.snapshot_for(username);
    data_store_.log_action(
        "play_card",
        username,
        "{\"cardId\":" + std::to_string(card_id) +
            ",\"card\":" + card_json(*card) + "}",
        result.ok,
        result.message,
        state,
        updated_state);
    if (result.ok) {
      if (updated_state.has_value() && updated_state->status == cruce::MatchStatus::Complete &&
          updated_state->winner.has_value()) {
        data_store_.record_completed_match(*updated_state);
        create_rematch_offer(*updated_state);
        server_.release_completed_match(updated_state->match_id);
        advance_ai();
        return action_json(result, updated_state);
      }
      advance_ai();
      return action_json(result, server_.snapshot_for(username));
    }
    return action_json(result, updated_state);
  }

  std::string invite(const std::map<std::string, std::string>& params) {
    const auto from = param(params, "from");
    const auto to = param(params, "to");
    int player_count = std::stoi(param(params, "players", "2"));
    if (player_count < 2 || player_count > 4) {
      return simple_json(false, "Choose a game for 2, 3, or 4 players.");
    }

    if (!is_online(from) || !is_online(to)) {
      return simple_json(false, "Both players must be online.");
    }
    if (from == to) {
      return simple_json(false, "You cannot invite yourself.");
    }

    const int room_id = std::stoi(param(params, "room", "0"));
    std::optional<int> waiting_room_id;
    if (room_id > 0) {
      auto* room = waiting_room_by_id(room_id);
      if (room == nullptr || !contains_text(room->players, from)) {
        return simple_json(false, "You are not in that waiting room.");
      }
      if (static_cast<int>(room->players.size()) >= room->player_count) {
        return simple_json(false, "Waiting room is already full.");
      }
      if (is_busy(to)) {
        return simple_json(false, "Invited player must be available.");
      }
      player_count = room->player_count;
      waiting_room_id = room->id;
    } else if (is_busy(from) || is_busy(to)) {
      return simple_json(false, "Both players must be available.");
    }

    for (const auto& invitation : invitations_) {
      if (invitation.to == to && invitation.status == "pending" &&
          invitation.waiting_room_id == waiting_room_id) {
        return simple_json(false, "Invitation already sent.");
      }
    }

    const int invitation_id = next_invitation_id_++;
    invitations_.push_back(Invitation{
        invitation_id,
        from,
        to,
        player_count,
        waiting_room_id,
    });
    notices_.erase(from);
    if (is_ai_player(to)) {
      const auto accepted = respond({
          {"username", to},
          {"invite", std::to_string(invitation_id)},
          {"accept", "1"},
      });
      advance_ai();
      return simple_json(
          true,
          "Invitation sent. " + to + " accepted automatically.");
    }
    advance_ai();
    return simple_json(
        true,
        "Invitation sent for a " + std::to_string(player_count) + "-player game.");
  }

  std::string select_target(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return simple_json(false, "User is not logged in.");
    }

    const int target_score = std::stoi(param(params, "score", "0"));
    if (!cruce::is_valid_target_score(target_score)) {
      return simple_json(false, "Target score must be 6, 11, or 21.");
    }

    const int requested_id = std::stoi(param(params, "selection", "0"));
    auto found = std::find_if(
        target_selections_.begin(),
        target_selections_.end(),
        [&](const TargetSelection& selection) {
          const bool belongs_to_user = contains_text(selection.players, username);
          return belongs_to_user &&
                 (requested_id == 0 || requested_id == selection.id);
        });
    if (found == target_selections_.end()) {
      return simple_json(false, "No target score selection is waiting.");
    }
    if (found->chooser != username) {
      return simple_json(false, "Only the randomly selected player can choose the target score.");
    }
    const bool all_online = std::all_of(
        found->players.begin(),
        found->players.end(),
        [&](const std::string& player) { return is_online(player); });
    if (!all_online) {
      target_selections_.erase(found);
      return simple_json(false, "All players must be online to start.");
    }

    const auto selection_players = found->players;
    const auto seated_players = seat_players_for_first_bidder(found->players, found->first_bidder);
    const auto room_code = server_.create_room(found->player_count, target_score);
    for (const auto& player : seated_players) {
      server_.join_room(
          room_code,
          cruce::server::JoinRequest{player, player, platforms_[player]});
    }

    const auto first_player = found->players.front();
    const auto players = found->players;
    const auto match = server_.snapshot_for(first_player);
    target_selections_.erase(found);
    const auto notice =
        "Target score " + std::to_string(target_score) + " selected. Match started.";
    for (const auto& player : players) {
      notices_[player] = notice;
    }

    if (!match.has_value()) {
      data_store_.log_action(
          "select_target",
          username,
          "{\"score\":" + std::to_string(target_score) +
              ",\"players\":" + string_array_json(selection_players) + "}",
          false,
          "Unable to start match.",
          std::nullopt,
          std::nullopt);
      return simple_json(false, "Unable to start match.");
    }
    data_store_.log_action(
        "select_target",
        username,
        "{\"score\":" + std::to_string(target_score) +
            ",\"players\":" + string_array_json(selection_players) + "}",
        true,
        notice,
        std::nullopt,
        match);
    advance_ai();
    return simple_json(true, notice);
  }

  std::string respond(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    const int invitation_id = std::stoi(param(params, "invite", "0"));
    const bool accepted = param(params, "accept", "0") == "1";

    auto found = std::find_if(
        invitations_.begin(),
        invitations_.end(),
        [&](const Invitation& invitation) {
          return invitation.id == invitation_id && invitation.to == username &&
                 invitation.status == "pending";
        });
    if (found == invitations_.end()) {
      return simple_json(false, "Invitation is no longer available.");
    }

    if (!accepted) {
      found->status = "declined";
      notices_[found->from] = username + " declined the invitation.";
      return simple_json(true, "Invitation declined.");
    }

    if (!is_online(found->from) || !is_online(found->to)) {
      found->status = "expired";
      return simple_json(false, "Both players must be online to start.");
    }

    if (found->waiting_room_id.has_value()) {
      auto* room = waiting_room_by_id(*found->waiting_room_id);
      if (room == nullptr) {
        found->status = "expired";
        return simple_json(false, "Waiting room is no longer available.");
      }
      if (is_busy(found->to)) {
        return simple_json(false, "You are already busy.");
      }

      room->players.push_back(found->to);
      room->messages.push_back(ChatMessage{"system", found->to + " joined the waiting room."});
      found->status = "accepted";
      attach_pending_lobby_invitations_to_room(*room, found->from);
      const auto response = fill_or_wait(*room);
      advance_ai();
      return response;
    }

    if (auto* room = joinable_waiting_room_for_lobby_invitation(*found)) {
      if (is_busy(found->to)) {
        return simple_json(false, "You are already busy.");
      }

      room->players.push_back(found->to);
      room->messages.push_back(ChatMessage{"system", found->to + " joined the waiting room."});
      found->waiting_room_id = room->id;
      found->status = "accepted";
      attach_pending_lobby_invitations_to_room(*room, found->from);
      const auto response = fill_or_wait(*room);
      advance_ai();
      return response;
    }

    if (is_busy(found->from) || is_busy(found->to)) {
      return simple_json(false, "Both players must be available.");
    }

    found->status = "accepted";
    std::vector<std::string> players{found->from, found->to};
    if (found->player_count == 2) {
      const auto response = create_target_selection(players, 2);
      advance_ai();
      return response;
    }

    WaitingRoom room;
    room.id = next_waiting_room_id_++;
    room.player_count = found->player_count;
    room.players = players;
    room.messages.push_back(ChatMessage{
        "system",
        "Waiting for " + std::to_string(found->player_count - 2) + " more player(s)."});
    waiting_rooms_.push_back(room);
    attach_pending_lobby_invitations_to_room(waiting_rooms_.back(), found->from);

    const auto notice =
        "Waiting room created for a " + std::to_string(found->player_count) +
        "-player game. Invite more players or chat while waiting.";
    notices_[found->from] = notice;
    notices_[found->to] = notice;
    advance_ai();
    return simple_json(true, notice);
  }

  std::string chat(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    auto message = param(params, "message");
    if (!is_online(username)) {
      return simple_json(false, "User is not logged in.");
    }
    auto* room = waiting_room_for(username);
    if (room == nullptr) {
      return simple_json(false, "You are not in a waiting room.");
    }
    if (message.empty()) {
      return simple_json(false, "Enter a chat message first.");
    }
    if (message.size() > 240) {
      message.resize(240);
    }

    room->messages.push_back(ChatMessage{username, message});
    return simple_json(true, "Message sent.");
  }

  std::string respond_rematch(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return simple_json(false, "User is not logged in.");
    }

    auto* offer = rematch_offer_for(username);
    if (offer == nullptr) {
      return simple_json(false, "No rematch is waiting.");
    }

    const bool accepted = param(params, "accept", "0") == "1";
    if (!accepted) {
      const auto players = offer->players;
      const auto notice = username + " declined the rematch. Returning to lobby.";
      erase_rematch_offer(offer->id);
      for (const auto& player : players) {
        notices_[player] = notice;
      }
      return simple_json(true, notice);
    }

    offer->accepted_by_player[username] = true;
    if (offer->accepted_by_player.size() == offer->players.size()) {
      const auto players = offer->players;
      const auto player_count = offer->player_count;
      erase_rematch_offer(offer->id);
      const auto response = create_target_selection(players, player_count);
      advance_ai();
      return response;
    }

    const auto notice = username + " accepted the rematch. Waiting for other player(s).";
    for (const auto& player : offer->players) {
      notices_[player] = notice;
    }
    advance_ai();
    return simple_json(true, notice);
  }

  std::string cancel_to_lobby(const std::map<std::string, std::string>& params) {
    const auto username = param(params, "username");
    if (!is_online(username)) {
      return simple_json(false, "User is not logged in.");
    }

    const auto notice = username + " returned to the lobby. The game was canceled.";
    if (const auto state = server_.snapshot_for(username)) {
      for (const auto& player : state->players) {
        notices_[player.player_id] = notice;
      }
      server_.cancel_match(state->match_id);
      data_store_.log_action(
          "cancel_match",
          username,
          "{}",
          true,
          notice,
          state,
          std::nullopt);
      return simple_json(true, notice);
    }

    if (auto* selection = target_selection_for_mutable(username)) {
      const auto players = selection->players;
      const auto selection_id = selection->id;
      target_selections_.erase(
          std::remove_if(
              target_selections_.begin(),
              target_selections_.end(),
              [selection_id](const TargetSelection& item) {
                return item.id == selection_id;
              }),
          target_selections_.end());
      for (const auto& player : players) {
        notices_[player] = notice;
      }
      return simple_json(true, notice);
    }

    if (auto* room = waiting_room_for(username)) {
      const auto players = room->players;
      const auto room_id = room->id;
      waiting_rooms_.erase(
          std::remove_if(
              waiting_rooms_.begin(),
              waiting_rooms_.end(),
              [room_id](const WaitingRoom& item) { return item.id == room_id; }),
          waiting_rooms_.end());
      expire_pending_invitations_for_room(room_id);
      for (const auto& player : players) {
        notices_[player] = notice;
      }
      return simple_json(true, notice);
    }

    if (auto* offer = rematch_offer_for(username)) {
      const auto players = offer->players;
      const auto offer_id = offer->id;
      erase_rematch_offer(offer_id);
      for (const auto& player : players) {
        notices_[player] = notice;
      }
      return simple_json(true, notice);
    }

    return simple_json(true, "Already in the lobby.");
  }

 private:
  void initialize_ai_players() {
    ai_players_ = {"ai_bot_1", "ai_bot_2", "ai_bot_3", "ai_bot_4"};
    for (const auto& bot : ai_players_) {
      online_users_.insert(bot);
      platforms_[bot] = cruce::ClientPlatform::Bot;
    }
  }

  bool is_ai_player(const std::string& username) const {
    return contains_text(ai_players_, username);
  }

  void advance_ai() {
    if (advancing_ai_) {
      return;
    }

    advancing_ai_ = true;
    for (int step = 0; step < 1000; ++step) {
      bool progressed = false;
      progressed = accept_pending_ai_invitation() || progressed;
      progressed = accept_pending_ai_rematch() || progressed;
      progressed = select_pending_ai_target() || progressed;
      progressed = play_one_ai_turn() || progressed;
      if (!progressed) {
        break;
      }
    }
    advancing_ai_ = false;
  }

  bool accept_pending_ai_invitation() {
    const auto found = std::find_if(
        invitations_.begin(),
        invitations_.end(),
        [&](const Invitation& invitation) {
          return invitation.status == "pending" && is_ai_player(invitation.to);
        });
    if (found == invitations_.end()) {
      return false;
    }

    const auto bot = found->to;
    const auto invite_id = found->id;
    data_store_.log_action(
        "ai_accept_invite",
        bot,
        "{\"invite\":" + std::to_string(invite_id) + "}",
        true,
        "AI accepted invitation.",
        std::nullopt,
        std::nullopt);
    respond({
        {"username", bot},
        {"invite", std::to_string(invite_id)},
        {"accept", "1"},
    });
    return true;
  }

  bool accept_pending_ai_rematch() {
    for (auto& offer : rematch_offers_) {
      for (const auto& bot : ai_players_) {
        if (contains_text(offer.players, bot) &&
            !offer.accepted_by_player.contains(bot)) {
          const auto offer_id = offer.id;
          data_store_.log_action(
              "ai_accept_rematch",
              bot,
              "{\"offer\":" + std::to_string(offer_id) + "}",
              true,
              "AI accepted rematch.",
              std::nullopt,
              std::nullopt);
          respond_rematch({{"username", bot}, {"accept", "1"}});
          return true;
        }
      }
    }
    return false;
  }

  bool select_pending_ai_target() {
    const auto found = std::find_if(
        target_selections_.begin(),
        target_selections_.end(),
        [&](const TargetSelection& selection) {
          return is_ai_player(selection.chooser);
        });
    if (found == target_selections_.end()) {
      return false;
    }

    const auto chooser = found->chooser;
    const std::vector<int> targets = {6, 11, 21};
    const int index = static_cast<int>(
        std::hash<std::string>{}(std::to_string(found->id) + chooser) % targets.size());
    select_target({{"username", chooser}, {"score", std::to_string(targets[index])}});
    return true;
  }

  bool play_one_ai_turn() {
    for (const auto& bot : ai_players_) {
      const auto state = server_.snapshot_for(bot);
      if (!state.has_value() ||
          !state->current_turn_player.has_value() ||
          *state->current_turn_player != bot) {
        continue;
      }

      if (state->status == cruce::MatchStatus::Bidding) {
        return submit_ai_bid(*state, bot);
      }
      if (state->status == cruce::MatchStatus::ChoosingTrump) {
        return submit_ai_trump(*state, bot);
      }
      if (state->status == cruce::MatchStatus::Playing) {
        return submit_ai_card(*state, bot);
      }
    }
    return false;
  }

  bool submit_ai_bid(const cruce::server::GameSnapshot& before, const std::string& bot) {
    const int bid_value = choose_ai_bid(before);
    const auto result = server_.submit_bid(bot, bid_value);
    const auto after = server_.snapshot_for(bot);
    data_store_.log_action(
        "ai_bid",
        bot,
        "{\"value\":" + std::to_string(bid_value) + "}",
        result.ok,
        result.message,
        before,
        after);
    return result.ok;
  }

  bool submit_ai_trump(const cruce::server::GameSnapshot& before, const std::string& bot) {
    const auto suit = choose_ai_trump(before);
    const auto result = server_.choose_trump(bot, suit);
    const auto after = server_.snapshot_for(bot);
    data_store_.log_action(
        "ai_choose_trump",
        bot,
        "{\"suit\":" + json_string(cruce::to_string(suit)) + "}",
        result.ok,
        result.message,
        before,
        after);
    return result.ok;
  }

  bool submit_ai_card(const cruce::server::GameSnapshot& before, const std::string& bot) {
    const auto card = choose_ai_card(before);
    if (!card.has_value()) {
      data_store_.log_action(
          "ai_play_card",
          bot,
          "{}",
          false,
          "AI found no legal card.",
          before,
          before);
      return false;
    }

    const auto result = server_.submit_card_play(bot, *card);
    const auto after = server_.snapshot_for(bot);
    data_store_.log_action(
        "ai_play_card",
        bot,
        "{\"cardId\":" + std::to_string(card->id()) +
            ",\"card\":" + card_json(*card) + "}",
        result.ok,
        result.message,
        before,
        after);
    if (result.ok && after.has_value() &&
        after->status == cruce::MatchStatus::Complete &&
        after->winner.has_value()) {
      data_store_.record_completed_match(*after);
      create_rematch_offer(*after);
      server_.release_completed_match(after->match_id);
    }
    return result.ok;
  }

  int choose_ai_bid(const cruce::server::GameSnapshot& state) const {
    return CruceHeuristicBot::choose_bid(state);
  }

  cruce::Suit choose_ai_trump(const cruce::server::GameSnapshot& state) const {
    return CruceHeuristicBot::choose_trump(state);
  }

  std::optional<cruce::Card> choose_ai_card(const cruce::server::GameSnapshot& state) const {
    return CruceHeuristicBot::choose_card(state);
  }

  static std::string param(
      const std::map<std::string, std::string>& params,
      const std::string& key,
      const std::string& fallback = "") {
    const auto found = params.find(key);
    return found == params.end() ? fallback : found->second;
  }

  bool is_online(const std::string& username) const {
    return (users_.exists(username) || is_ai_player(username)) &&
           online_users_.contains(username);
  }

  bool is_busy(const std::string& username) const {
    return server_.snapshot_for(username).has_value() ||
           target_selection_for(username) != nullptr ||
           waiting_room_for(username) != nullptr ||
           rematch_offer_for(username) != nullptr;
  }

  const TargetSelection* target_selection_for(const std::string& username) const {
    const auto found = std::find_if(
        target_selections_.begin(),
        target_selections_.end(),
        [&](const TargetSelection& selection) {
          return contains_text(selection.players, username);
        });
    return found == target_selections_.end() ? nullptr : &*found;
  }

  TargetSelection* target_selection_for_mutable(const std::string& username) {
    const auto found = std::find_if(
        target_selections_.begin(),
        target_selections_.end(),
        [&](const TargetSelection& selection) {
          return contains_text(selection.players, username);
        });
    return found == target_selections_.end() ? nullptr : &*found;
  }

  WaitingRoom* waiting_room_by_id(int room_id) {
    const auto found = std::find_if(
        waiting_rooms_.begin(),
        waiting_rooms_.end(),
        [room_id](const WaitingRoom& room) { return room.id == room_id; });
    return found == waiting_rooms_.end() ? nullptr : &*found;
  }

  const WaitingRoom* waiting_room_for(const std::string& username) const {
    const auto found = std::find_if(
        waiting_rooms_.begin(),
        waiting_rooms_.end(),
        [&](const WaitingRoom& room) { return contains_text(room.players, username); });
    return found == waiting_rooms_.end() ? nullptr : &*found;
  }

  WaitingRoom* waiting_room_for(const std::string& username) {
    const auto found = std::find_if(
        waiting_rooms_.begin(),
        waiting_rooms_.end(),
        [&](const WaitingRoom& room) { return contains_text(room.players, username); });
    return found == waiting_rooms_.end() ? nullptr : &*found;
  }

  WaitingRoom* joinable_waiting_room_for_lobby_invitation(const Invitation& invitation) {
    if (invitation.player_count <= 2) {
      return nullptr;
    }

    const auto found = std::find_if(
        waiting_rooms_.begin(),
        waiting_rooms_.end(),
        [&](const WaitingRoom& room) {
          return room.player_count == invitation.player_count &&
                 static_cast<int>(room.players.size()) < room.player_count &&
                 contains_text(room.players, invitation.from) &&
                 !contains_text(room.players, invitation.to);
        });
    return found == waiting_rooms_.end() ? nullptr : &*found;
  }

  void attach_pending_lobby_invitations_to_room(
      const WaitingRoom& room,
      const std::string& inviter) {
    for (auto& invitation : invitations_) {
      if (invitation.status == "pending" &&
          !invitation.waiting_room_id.has_value() &&
          invitation.from == inviter &&
          invitation.player_count == room.player_count &&
          !contains_text(room.players, invitation.to)) {
        invitation.waiting_room_id = room.id;
      }
    }
  }

  void expire_pending_invitations_for_room(int room_id) {
    for (auto& invitation : invitations_) {
      if (invitation.waiting_room_id == room_id &&
          invitation.status == "pending") {
        invitation.status = "expired";
      }
    }
  }

  const RematchOffer* rematch_offer_for(const std::string& username) const {
    const auto found = std::find_if(
        rematch_offers_.begin(),
        rematch_offers_.end(),
        [&](const RematchOffer& offer) { return contains_text(offer.players, username); });
    return found == rematch_offers_.end() ? nullptr : &*found;
  }

  RematchOffer* rematch_offer_for(const std::string& username) {
    const auto found = std::find_if(
        rematch_offers_.begin(),
        rematch_offers_.end(),
        [&](const RematchOffer& offer) { return contains_text(offer.players, username); });
    return found == rematch_offers_.end() ? nullptr : &*found;
  }

  void erase_rematch_offer(int offer_id) {
    rematch_offers_.erase(
        std::remove_if(
            rematch_offers_.begin(),
            rematch_offers_.end(),
            [offer_id](const RematchOffer& offer) { return offer.id == offer_id; }),
        rematch_offers_.end());
  }

  void create_rematch_offer(const cruce::server::GameSnapshot& state) {
    std::vector<std::string> players;
    players.reserve(state.players.size());
    for (const auto& player : state.players) {
      players.push_back(player.player_id);
    }

    RematchOffer offer;
    offer.id = next_rematch_offer_id_++;
    offer.player_count = static_cast<int>(players.size());
    offer.players = players;
    offer.winner = state.winner.value_or("A player");
    rematch_offers_.push_back(offer);

    const auto notice = offer.winner + " won the match. Rematch?";
    for (const auto& player : players) {
      notices_[player] = notice;
    }
  }

  std::string fill_or_wait(const WaitingRoom& room) {
    const int remaining =
        room.player_count - static_cast<int>(room.players.size());
    if (remaining <= 0) {
      const auto players = room.players;
      const auto player_count = room.player_count;
      const auto room_id = room.id;
      waiting_rooms_.erase(
          std::remove_if(
              waiting_rooms_.begin(),
              waiting_rooms_.end(),
              [room_id](const WaitingRoom& waiting_room) {
                return waiting_room.id == room_id;
              }),
          waiting_rooms_.end());
      expire_pending_invitations_for_room(room_id);
      return create_target_selection(players, player_count);
    }

    const auto notice =
        "Waiting for " + std::to_string(remaining) + " more player(s).";
    for (const auto& player : room.players) {
      notices_[player] = notice;
    }
    return simple_json(true, notice);
  }

  std::string create_target_selection(
      const std::vector<std::string>& players,
      int player_count) {
    if (static_cast<int>(players.size()) != player_count) {
      return simple_json(false, "Not enough players to start.");
    }

    std::random_device random_device;
    const auto chooser = players[random_device() % players.size()];
    const auto first_bidder = players[random_device() % players.size()];

    target_selections_.push_back(TargetSelection{
        next_target_selection_id_++,
        player_count,
        players,
        chooser,
        first_bidder,
    });
    const auto notice = "Room is full. " + chooser + " selects the target score.";
    for (const auto& player : players) {
      notices_[player] = notice;
    }
    return simple_json(true, notice);
  }

  static std::vector<std::string> seat_players_for_first_bidder(
      const std::vector<std::string>& players,
      const std::string& first_bidder) {
    std::vector<std::string> seated;
    seated.reserve(players.size());

    const auto dealer = std::find_if(
        players.begin(),
        players.end(),
        [&](const std::string& player) { return player != first_bidder; });
    if (dealer != players.end()) {
      seated.push_back(*dealer);
    }
    seated.push_back(first_bidder);
    for (const auto& player : players) {
      if (!contains_text(seated, player)) {
        seated.push_back(player);
      }
    }
    return seated;
  }

  static std::string simple_json(bool ok, const std::string& message) {
    return "{\"ok\":" + std::string(ok ? "true" : "false") +
           ",\"message\":" + json_string(message) + "}";
  }

  static std::string auth_json(
      const cruce::ValidationResult& result,
      const std::string& username) {
    return "{\"ok\":" + std::string(result.ok ? "true" : "false") +
           ",\"message\":" + json_string(result.message) +
           ",\"username\":" + json_string(result.ok ? username : "") + "}";
  }

  cruce::server::UserStore users_;
  GameDataStore data_store_;
  cruce::server::GameServer server_;
  std::vector<std::string> ai_players_;
  std::set<std::string> online_users_;
  std::map<std::string, cruce::ClientPlatform> platforms_;
  std::vector<Invitation> invitations_;
  std::vector<TargetSelection> target_selections_;
  std::vector<WaitingRoom> waiting_rooms_;
  std::vector<RematchOffer> rematch_offers_;
  std::map<std::string, std::string> notices_;
  int next_invitation_id_ = 1;
  int next_target_selection_id_ = 1;
  int next_waiting_room_id_ = 1;
  int next_rematch_offer_id_ = 1;
  bool advancing_ai_ = false;
};

std::optional<std::string> read_file_bytes(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    return std::nullopt;
  }
  std::ostringstream out;
  out << input.rdbuf();
  return out.str();
}

std::string content_type_for(const std::filesystem::path& path) {
  if (path.extension() == ".png") {
    return "image/png";
  }
  if (path.extension() == ".jpg" || path.extension() == ".jpeg") {
    return "image/jpeg";
  }
  return "application/octet-stream";
}

void send_response(
    SOCKET client,
    const std::string& body,
    const std::string& content_type,
    int status = 200,
    const std::string& status_text = "OK") {
  std::ostringstream response;
  response << "HTTP/1.1 " << status << " " << status_text << "\r\n"
           << "Content-Type: " << content_type << "\r\n"
           << "Access-Control-Allow-Origin: *\r\n"
           << "Content-Length: " << body.size() << "\r\n"
           << "Connection: close\r\n\r\n"
           << body;
  const auto text = response.str();
  send(client, text.c_str(), static_cast<int>(text.size()), 0);
}

void route_request(SOCKET client, LobbyState& lobby, const std::string& method, const std::string& target) {
  if (method != "GET") {
    send_response(client, "{\"ok\":false,\"message\":\"Only GET is supported.\"}", "application/json", 405, "Method Not Allowed");
    return;
  }

  const auto path = path_only(target);
  const auto params = query_params(target);
  if (path == "/api/login") {
    send_response(client, lobby.login(params), "application/json");
  } else if (path == "/api/register") {
    send_response(client, lobby.register_user(params), "application/json");
  } else if (path == "/api/reconnect") {
    send_response(client, lobby.reconnect(params), "application/json");
  } else if (path == "/api/lobby") {
    send_response(client, lobby.lobby_json(params), "application/json");
  } else if (path == "/api/lobby.txt") {
    send_response(client, lobby.lobby_text(params), "text/plain; charset=utf-8");
  } else if (path == "/api/match") {
    send_response(client, lobby.match_json(params), "application/json");
  } else if (path == "/api/match.txt") {
    send_response(client, lobby.match_text(params), "text/plain; charset=utf-8");
  } else if (path == "/api/bid") {
    send_response(client, lobby.bid(params), "application/json");
  } else if (path == "/api/trump") {
    send_response(client, lobby.choose_trump(params), "application/json");
  } else if (path == "/api/play") {
    send_response(client, lobby.play_card(params), "application/json");
  } else if (path == "/api/invite") {
    send_response(client, lobby.invite(params), "application/json");
  } else if (path == "/api/target") {
    send_response(client, lobby.select_target(params), "application/json");
  } else if (path == "/api/respond") {
    send_response(client, lobby.respond(params), "application/json");
  } else if (path == "/api/chat") {
    send_response(client, lobby.chat(params), "application/json");
  } else if (path == "/api/rematch") {
    send_response(client, lobby.respond_rematch(params), "application/json");
  } else if (path == "/api/cancel") {
    send_response(client, lobby.cancel_to_lobby(params), "application/json");
  } else if (path.starts_with("/assets/cards/")) {
    const auto file_name = path.substr(std::string("/assets/cards/").size());
    if (file_name.find("..") != std::string::npos || file_name.find('/') != std::string::npos) {
      send_response(client, "Not found", "text/plain", 404, "Not Found");
      return;
    }
    const std::filesystem::path file_path = std::filesystem::path("assets/cards") / file_name;
    const auto file = read_file_bytes(file_path);
    if (!file.has_value()) {
      send_response(client, "Not found", "text/plain", 404, "Not Found");
      return;
    }
    send_response(client, *file, content_type_for(file_path));
  } else {
    send_response(client, html_page(), "text/html; charset=utf-8");
  }
}

int run_server(unsigned short port) {
  WSADATA data;
  if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
    std::cerr << "WSAStartup failed.\n";
    return 1;
  }

  SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listener == INVALID_SOCKET) {
    std::cerr << "Unable to create socket.\n";
    WSACleanup();
    return 1;
  }

  BOOL reuse = TRUE;
  setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
    std::cerr << "Unable to bind to 0.0.0.0:" << port << ".\n";
    closesocket(listener);
    WSACleanup();
    return 1;
  }

  if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
    std::cerr << "Unable to listen on socket.\n";
    closesocket(listener);
    WSACleanup();
    return 1;
  }

  LobbyState lobby{"data/users.db"};
  std::cout << "Cruce local server running at http://127.0.0.1:" << port << "\n";
  std::cout << "LAN clients can connect to http://<this-pc-ip>:" << port << "\n";
  std::cout << "Users can log in or register from the clients.\n";

  while (keep_running) {
    SOCKET client = accept(listener, nullptr, nullptr);
    if (client == INVALID_SOCKET) {
      continue;
    }

    char buffer[8192]{};
    const int received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (received > 0) {
      std::istringstream request(std::string(buffer, static_cast<std::size_t>(received)));
      std::string method;
      std::string target;
      request >> method >> target;
      route_request(client, lobby, method, target);
    }
    closesocket(client);
  }

  closesocket(listener);
  WSACleanup();
  return 0;
}

int parse_positive_int(const char* value, const std::string& name) {
  char* end = nullptr;
  const long parsed = std::strtol(value, &end, 10);
  if (end == value || *end != '\0' || parsed <= 0 || parsed > 1000000) {
    throw std::invalid_argument("Invalid " + name + ": " + value);
  }
  return static_cast<int>(parsed);
}

std::uint32_t parse_seed(const char* value) {
  char* end = nullptr;
  const unsigned long parsed = std::strtoul(value, &end, 10);
  if (end == value || *end != '\0') {
    throw std::invalid_argument(std::string("Invalid seed: ") + value);
  }
  return static_cast<std::uint32_t>(parsed);
}

SelfPlayOptions parse_self_play_options(int argc, char** argv) {
  SelfPlayOptions options;
  for (int index = 1; index < argc; ++index) {
    const std::string arg = argv[index];
    if (arg == "--self-play") {
      if (index + 1 < argc && argv[index + 1][0] != '-') {
        options.games = parse_positive_int(argv[++index], "self-play game count");
      }
    } else if (arg == "--players") {
      if (index + 1 >= argc) {
        throw std::invalid_argument("--players requires a value.");
      }
      options.player_count = parse_positive_int(argv[++index], "player count");
      if (options.player_count < 2 || options.player_count > 4) {
        throw std::invalid_argument("--players must be 2, 3, or 4.");
      }
    } else if (arg == "--target") {
      if (index + 1 >= argc) {
        throw std::invalid_argument("--target requires a value.");
      }
      options.target_score = parse_positive_int(argv[++index], "target score");
      if (!cruce::is_valid_target_score(options.target_score)) {
        throw std::invalid_argument("--target must be 6, 11, or 21.");
      }
    } else if (arg == "--seed") {
      if (index + 1 >= argc) {
        throw std::invalid_argument("--seed requires a value.");
      }
      options.seed = parse_seed(argv[++index]);
    } else if (arg == "--events") {
      if (index + 1 >= argc) {
        throw std::invalid_argument("--events requires a path.");
      }
      options.events_path = argv[++index];
    } else if (arg == "--history") {
      if (index + 1 >= argc) {
        throw std::invalid_argument("--history requires a path.");
      }
      options.history_path = argv[++index];
    } else if (arg == "--help" || arg == "-h") {
      throw std::invalid_argument(
          "usage: cruce_server_app [port]\n"
          "       cruce_server_app --self-play [games] [--players 2|3|4] "
          "[--target 6|11|21] [--seed n] [--events path] [--history path]");
    }
  }
  return options;
}

int run_self_play(const SelfPlayOptions& options) {
  GameDataStore data_store{options.events_path, options.history_path};
  SelfPlaySimulator simulator{data_store, options.seed};
  std::cout << "Generating " << options.games << " self-play game(s)";
  if (options.player_count >= 2) {
    std::cout << " with " << options.player_count << " player(s)";
  } else {
    std::cout << " with mixed player counts";
  }
  if (cruce::is_valid_target_score(options.target_score)) {
    std::cout << " to " << options.target_score << " points";
  } else {
    std::cout << " with mixed target scores";
  }
  std::cout << ".\n";
  std::cout << "Move log: " << options.events_path.string() << "\n";
  std::cout << "History: " << options.history_path.string() << "\n";

  const auto stats = simulator.run(options);
  std::cout << "Self-play complete: " << stats.completed << "/" << stats.requested
            << " games completed, " << stats.failed << " failed, "
            << stats.actions << " actions logged.\n";
  return stats.failed == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
  std::signal(SIGINT, handle_signal);
  if (argc > 1 && std::string(argv[1]) == "--self-play") {
    try {
      return run_self_play(parse_self_play_options(argc, argv));
    } catch (const std::exception& error) {
      std::cerr << error.what() << "\n";
      return 2;
    }
  }

  const unsigned short port =
      argc > 1 ? static_cast<unsigned short>(std::strtoul(argv[1], nullptr, 10)) : 8080;
  return run_server(port);
}
