#include "core/types.hpp"

#include <stdexcept>

namespace cruce {

namespace {

int suit_index(Suit suit) {
  switch (suit) {
    case Suit::Hearts:
      return 0;
    case Suit::Diamonds:
      return 1;
    case Suit::Clubs:
      return 2;
    case Suit::Spades:
      return 3;
  }
  throw std::logic_error("unknown suit");
}

int rank_index(Rank rank) {
  switch (rank) {
    case Rank::Ace:
      return 0;
    case Rank::Ten:
      return 1;
    case Rank::King:
      return 2;
    case Rank::Queen:
      return 3;
    case Rank::Jack:
      return 4;
    case Rank::Nine:
      return 5;
  }
  throw std::logic_error("unknown rank");
}

}  // namespace

ValidationResult ValidationResult::success(std::string message) {
  return {true, std::move(message)};
}

ValidationResult ValidationResult::failure(std::string message) {
  return {false, std::move(message)};
}

int Card::id() const {
  return suit_index(suit) * 10 + rank_index(rank);
}

int Card::point_value() const {
  return cruce::point_value(rank);
}

int Card::strength() const {
  return rank_strength(rank);
}

std::string Card::label() const {
  return to_string(rank) + " of " + to_string(suit);
}

bool operator==(const Card& left, const Card& right) {
  return left.suit == right.suit && left.rank == right.rank;
}

bool operator!=(const Card& left, const Card& right) {
  return !(left == right);
}

bool operator<(const Card& left, const Card& right) {
  return left.id() < right.id();
}

bool Bid::is_higher_than(const std::optional<Bid>& other) const {
  if (passed) {
    return false;
  }
  return !other.has_value() || value > other->value;
}

std::vector<Suit> all_suits() {
  return {Suit::Hearts, Suit::Diamonds, Suit::Clubs, Suit::Spades};
}

std::vector<Rank> all_ranks() {
  return {Rank::Ace, Rank::Ten, Rank::King, Rank::Queen, Rank::Jack, Rank::Nine};
}

std::string to_string(Suit suit) {
  switch (suit) {
    case Suit::Hearts:
      return "Hearts";
    case Suit::Diamonds:
      return "Diamonds";
    case Suit::Clubs:
      return "Clubs";
    case Suit::Spades:
      return "Spades";
  }
  throw std::logic_error("unknown suit");
}

std::string to_string(Rank rank) {
  switch (rank) {
    case Rank::Ace:
      return "Ace";
    case Rank::Ten:
      return "Ten";
    case Rank::King:
      return "King";
    case Rank::Queen:
      return "Queen";
    case Rank::Jack:
      return "Jack";
    case Rank::Nine:
      return "Nine";
  }
  throw std::logic_error("unknown rank");
}

std::string to_string(ClientPlatform platform) {
  switch (platform) {
    case ClientPlatform::Mobile:
      return "Mobile";
    case ClientPlatform::WindowsDesktop:
      return "WindowsDesktop";
    case ClientPlatform::Web:
      return "Web";
    case ClientPlatform::Bot:
      return "Bot";
  }
  throw std::logic_error("unknown client platform");
}

std::string to_string(MatchStatus status) {
  switch (status) {
    case MatchStatus::WaitingForPlayers:
      return "WaitingForPlayers";
    case MatchStatus::Bidding:
      return "Bidding";
    case MatchStatus::ChoosingTrump:
      return "ChoosingTrump";
    case MatchStatus::Playing:
      return "Playing";
    case MatchStatus::Complete:
      return "Complete";
  }
  throw std::logic_error("unknown match status");
}

int point_value(Rank rank) {
  switch (rank) {
    case Rank::Ace:
      return 11;
    case Rank::Ten:
      return 10;
    case Rank::King:
      return 4;
    case Rank::Queen:
      return 3;
    case Rank::Jack:
      return 2;
    case Rank::Nine:
      return 0;
  }
  throw std::logic_error("unknown rank");
}

int rank_strength(Rank rank) {
  switch (rank) {
    case Rank::Ace:
      return 6;
    case Rank::Ten:
      return 5;
    case Rank::King:
      return 4;
    case Rank::Queen:
      return 3;
    case Rank::Jack:
      return 2;
    case Rank::Nine:
      return 1;
  }
  throw std::logic_error("unknown rank");
}

bool is_valid_target_score(int target_score) {
  return target_score == 6 || target_score == 11 || target_score == 21;
}

}  // namespace cruce
