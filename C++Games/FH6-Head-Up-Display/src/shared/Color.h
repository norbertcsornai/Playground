#pragma once

#include <algorithm>
#include <cstdint>

namespace fh6 {

struct Color {
  std::uint8_t r{0};
  std::uint8_t g{0};
  std::uint8_t b{0};
  std::uint8_t a{255};

  [[nodiscard]] double redDominance() const {
    return static_cast<double>(r) - (static_cast<double>(g) + static_cast<double>(b)) / 2.0;
  }

  [[nodiscard]] double brightness() const {
    return (0.2126 * static_cast<double>(r)) + (0.7152 * static_cast<double>(g)) +
           (0.0722 * static_cast<double>(b));
  }
};

struct ColorThreshold {
  int minRed{0};
  int minGreen{0};
  int minBlue{0};
  int maxRed{255};
  int maxGreen{255};
  int maxBlue{255};
  double minBrightness{0.0};
  double minRedDominance{0.0};

  [[nodiscard]] bool matches(const Color& color) const {
    return color.r >= minRed && color.r <= maxRed && color.g >= minGreen && color.g <= maxGreen &&
           color.b >= minBlue && color.b <= maxBlue && color.brightness() >= minBrightness &&
           color.redDominance() >= minRedDominance;
  }
};

struct ColorThresholds {
  ColorThreshold white{
      115, 115, 115,
      255, 255, 255,
      120.0,
      -85.0,
  };
  ColorThreshold red{
      150, 0, 20,
      255, 115, 190,
      55.0,
      55.0,
  };
};

}  // namespace fh6
