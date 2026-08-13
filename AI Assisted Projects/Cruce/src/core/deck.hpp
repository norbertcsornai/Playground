#pragma once

#include "core/types.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace cruce {

class Deck {
 public:
  Deck() = default;
  explicit Deck(std::vector<Card> cards);

  static Deck standard_24();

  void shuffle(std::uint32_t seed);
  std::vector<Card> deal(std::size_t count);
  std::optional<Card> draw();

  bool empty() const;
  std::size_t size() const;
  const std::vector<Card>& cards() const;

 private:
  std::vector<Card> cards_;
};

}  // namespace cruce
