#pragma once

#include <optional>
#include <string>
#include <vector>

namespace cruce {

using PlayerId = std::string;
using OwnerId = std::string;

enum class Suit {
  Hearts,
  Diamonds,
  Clubs,
  Spades,
};

enum class Rank {
  Ace,
  Ten,
  King,
  Queen,
  Jack,
  Nine,
};

enum class ClientPlatform {
  Mobile,
  WindowsDesktop,
  Web,
  Bot,
};

enum class MatchStatus {
  WaitingForPlayers,
  Bidding,
  ChoosingTrump,
  Playing,
  Complete,
};

struct ValidationResult {
  bool ok;
  std::string message;

  static ValidationResult success(std::string message = "ok");
  static ValidationResult failure(std::string message);
  explicit operator bool() const { return ok; }
};

struct Card {
  Suit suit;
  Rank rank;

  int id() const;
  int point_value() const;
  int strength() const;
  std::string label() const;
};

bool operator==(const Card& left, const Card& right);
bool operator!=(const Card& left, const Card& right);
bool operator<(const Card& left, const Card& right);

struct PlayedCard {
  PlayerId player_id;
  int seat_index;
  Card card;
};

struct Bid {
  PlayerId player_id;
  int value = 0;
  bool passed = true;

  bool is_higher_than(const std::optional<Bid>& other) const;
};

std::vector<Suit> all_suits();
std::vector<Rank> all_ranks();

std::string to_string(Suit suit);
std::string to_string(Rank rank);
std::string to_string(ClientPlatform platform);
std::string to_string(MatchStatus status);

int point_value(Rank rank);
int rank_strength(Rank rank);
bool is_valid_target_score(int target_score);

}  // namespace cruce
