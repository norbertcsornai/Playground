#pragma once  // Prevents this header from being included more than once.

#include <algorithm>  // Imports the algorithm standard library declarations used in this file.
#include <cstdint>  // Imports the cstdint standard library declarations used in this file.

namespace fh6 {  // Places the following declarations inside namespace fh6.

struct Color {  // Declares the Color value type and fields.
  std::uint8_t r{0};  // Declares r and initializes it with 0.
  std::uint8_t g{0};  // Declares g and initializes it with 0.
  std::uint8_t b{0};  // Declares b and initializes it with 0.
  std::uint8_t a{255};  // Declares a and initializes it with 255.

  [[nodiscard]] double redDominance() const {  // Begins function redDominance.
    return static_cast<double>(r) - (static_cast<double>(g) + static_cast<double>(b)) / 2.0;  // Returns static_cast<double>(r) - (static_cast<double>(g) + static_cast<double>(b)) / 2.0 to the caller.
  }  // Ends the current code block.

  [[nodiscard]] double brightness() const {  // Begins function brightness.
    return (0.2126 * static_cast<double>(r)) + (0.7152 * static_cast<double>(g)) +  // Continues the surrounding declaration or control-flow expression.
           (0.0722 * static_cast<double>(b));  // Executes (0.0722 * static_cast<double>(b)).
  }  // Ends the current code block.
};  // Ends the current type, struct, or initializer declaration.

struct ColorThreshold {  // Declares the ColorThreshold value type and fields.
  int minRed{0};  // Declares minRed and initializes it with 0.
  int minGreen{0};  // Declares minGreen and initializes it with 0.
  int minBlue{0};  // Declares minBlue and initializes it with 0.
  int maxRed{255};  // Declares maxRed and initializes it with 255.
  int maxGreen{255};  // Declares maxGreen and initializes it with 255.
  int maxBlue{255};  // Declares maxBlue and initializes it with 255.
  double minBrightness{0.0};  // Declares minBrightness and initializes it with 0.0.
  double minRedDominance{0.0};  // Declares minRedDominance and initializes it with 0.0.

  [[nodiscard]] bool matches(const Color& color) const {  // Begins function matches.
    return color.r >= minRed && color.r <= maxRed && color.g >= minGreen && color.g <= maxGreen &&  // Continues the surrounding declaration or control-flow expression.
           color.b >= minBlue && color.b <= maxBlue && color.brightness() >= minBrightness &&  // Continues the surrounding declaration or control-flow expression.
           color.redDominance() >= minRedDominance;  // Sets color.redDominance() > to minRedDominance.
  }  // Ends the current code block.
};  // Ends the current type, struct, or initializer declaration.

struct ColorThresholds {  // Declares the ColorThresholds value type and fields.
  ColorThreshold white{  // Starts a multi-line initializer or scope for ColorThreshold white.
      115, 115, 115,  // Supplies 115, 115, 115 to the surrounding call or initializer.
      255, 255, 255,  // Supplies 255, 255, 255 to the surrounding call or initializer.
      120.0,  // Supplies 120.0 to the surrounding call or initializer.
      -85.0,  // Supplies -85.0 to the surrounding call or initializer.
  };  // Ends the current type, struct, or initializer declaration.
  ColorThreshold red{  // Starts a multi-line initializer or scope for ColorThreshold red.
      150, 0, 20,  // Supplies 150, 0, 20 to the surrounding call or initializer.
      255, 115, 190,  // Supplies 255, 115, 190 to the surrounding call or initializer.
      55.0,  // Supplies 55.0 to the surrounding call or initializer.
      55.0,  // Supplies 55.0 to the surrounding call or initializer.
  };  // Ends the current type, struct, or initializer declaration.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
