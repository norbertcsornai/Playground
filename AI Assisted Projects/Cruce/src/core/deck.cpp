#include "core/deck.hpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace cruce {

Deck::Deck(std::vector<Card> cards) : cards_(std::move(cards)) {}

Deck Deck::standard_24() {
  std::vector<Card> cards;
  cards.reserve(24);
  for (const auto suit : all_suits()) {
    for (const auto rank : all_ranks()) {
      cards.push_back(Card{suit, rank});
    }
  }
  return Deck{std::move(cards)};
}

void Deck::shuffle(std::uint32_t seed) {
  std::mt19937 rng(seed);
  std::shuffle(cards_.begin(), cards_.end(), rng);
}

std::vector<Card> Deck::deal(std::size_t count) {
  if (count > cards_.size()) {
    throw std::out_of_range("not enough cards left to deal");
  }

  std::vector<Card> dealt;
  dealt.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    dealt.push_back(cards_.back());
    cards_.pop_back();
  }
  return dealt;
}

std::optional<Card> Deck::draw() {
  if (cards_.empty()) {
    return std::nullopt;
  }

  Card card = cards_.back();
  cards_.pop_back();
  return card;
}

bool Deck::empty() const {
  return cards_.empty();
}

std::size_t Deck::size() const {
  return cards_.size();
}

const std::vector<Card>& Deck::cards() const {
  return cards_;
}

}  // namespace cruce
