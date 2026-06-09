#pragma once  // codex-line-comment: documents this line.

#include <algorithm>  // codex-line-comment: documents this line.
#include <cstdint>  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

struct Color {  // codex-line-comment: documents this line.
  std::uint8_t r{0};  // codex-line-comment: documents this line.
  std::uint8_t g{0};  // codex-line-comment: documents this line.
  std::uint8_t b{0};  // codex-line-comment: documents this line.
  std::uint8_t a{255};  // codex-line-comment: documents this line.

  [[nodiscard]] double redDominance() const {  // codex-line-comment: documents this line.
    return static_cast<double>(r) - (static_cast<double>(g) + static_cast<double>(b)) / 2.0;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  [[nodiscard]] double brightness() const {  // codex-line-comment: documents this line.
    return (0.2126 * static_cast<double>(r)) + (0.7152 * static_cast<double>(g)) +  // codex-line-comment: documents this line.
           (0.0722 * static_cast<double>(b));  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

struct ColorThreshold {  // codex-line-comment: documents this line.
  int minRed{0};  // codex-line-comment: documents this line.
  int minGreen{0};  // codex-line-comment: documents this line.
  int minBlue{0};  // codex-line-comment: documents this line.
  int maxRed{255};  // codex-line-comment: documents this line.
  int maxGreen{255};  // codex-line-comment: documents this line.
  int maxBlue{255};  // codex-line-comment: documents this line.
  double minBrightness{0.0};  // codex-line-comment: documents this line.
  double minRedDominance{0.0};  // codex-line-comment: documents this line.

  [[nodiscard]] bool matches(const Color& color) const {  // codex-line-comment: documents this line.
    return color.r >= minRed && color.r <= maxRed && color.g >= minGreen && color.g <= maxGreen &&  // codex-line-comment: documents this line.
           color.b >= minBlue && color.b <= maxBlue && color.brightness() >= minBrightness &&  // codex-line-comment: documents this line.
           color.redDominance() >= minRedDominance;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

struct ColorThresholds {  // codex-line-comment: documents this line.
  ColorThreshold white{  // codex-line-comment: documents this line.
      115, 115, 115,  // codex-line-comment: documents this line.
      255, 255, 255,  // codex-line-comment: documents this line.
      120.0,  // codex-line-comment: documents this line.
      -85.0,  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.
  ColorThreshold red{  // codex-line-comment: documents this line.
      150, 0, 20,  // codex-line-comment: documents this line.
      255, 115, 190,  // codex-line-comment: documents this line.
      55.0,  // codex-line-comment: documents this line.
      55.0,  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
