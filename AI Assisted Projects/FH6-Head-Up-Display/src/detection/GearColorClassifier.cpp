#include "detection/GearColorClassifier.h"  // Imports project declarations from detection/GearColorClassifier.h.

#include <algorithm>  // Imports the algorithm standard library declarations used in this file.
#include <cmath>  // Imports the cmath standard library declarations used in this file.

namespace fh6 {  // Places the following declarations inside namespace fh6.

GearColorClassifier::GearColorClassifier(ColorThresholds thresholds) : thresholds_(thresholds) {}  // Finishes this initializer entry for the surrounding aggregate.

GearColorState GearColorClassifier::classify(const FrameRegion& region) const {  // Implements GearColorClassifier::classify.
  if (region.empty()) {  // Guards the following work behind the condition region.empty().
    return GearColorState::Unknown;  // Returns GearColorState::Unknown to the caller.
  }  // Ends the current code block.

  const int side = std::min(region.bounds().width, region.bounds().height);  // Sets const int side to std::min(region.bounds().width, region.bounds().height).
  const auto widgetStats = calculateWidgetColorStats(region);  // Sets const auto widgetStats to calculateWidgetColorStats(region).

  if (hasWhiteGearSignal(widgetStats.white, side)) {  // Guards the following work behind the condition hasWhiteGearSignal(widgetStats.white, side).
    return GearColorState::White;  // Returns GearColorState::White to the caller.
  }  // Ends the current code block.

  if (hasRedGearAndRing(widgetStats.red, side)) {  // Guards the following work behind the condition hasRedGearAndRing(widgetStats.red, side).
    return GearColorState::Red;  // Returns GearColorState::Red to the caller.
  }  // Ends the current code block.

  const auto whiteStats = calculateMatchStats(region, thresholds_.white);  // Sets const auto whiteStats to calculateMatchStats(region, thresholds_.white).

  if (hasEnoughGlyphPixels(whiteStats)) {  // Guards the following work behind the condition hasEnoughGlyphPixels(whiteStats).
    return GearColorState::White;  // Returns GearColorState::White to the caller.
  }  // Ends the current code block.

  return GearColorState::Unknown;  // Returns GearColorState::Unknown to the caller.
}  // Ends the current code block.

void GearColorClassifier::updateThresholds(const ColorThresholds& thresholds) {  // Implements GearColorClassifier::updateThresholds.
  thresholds_ = thresholds;  // Sets thresholds_ to thresholds.
}  // Ends the current code block.

GearColorClassifier::MatchStats GearColorClassifier::calculateMatchStats(  // Implements GearColorClassifier::calculateMatchStats.
    const FrameRegion& region, const ColorThreshold& target) const {  // Starts a multi-line initializer or scope for const FrameRegion& region, const ColorThreshold& target) const.
  const auto pixels = region.pixels();  // Sets const auto pixels to region.pixels().
  if (pixels.empty()) {  // Guards the following work behind the condition pixels.empty().
    return {};  // Returns {} to the caller.
  }  // Ends the current code block.

  int matches = 0;  // Sets int matches to 0.
  for (const auto& pixel : pixels) {  // Iterates with loop control const auto& pixel : pixels.
    if (target.matches(pixel)) {  // Guards the following work behind the condition target.matches(pixel).
      ++matches;  // Increments matches by one.
    }  // Ends the current code block.
  }  // Ends the current code block.

  return MatchStats{  // Starts returning a MatchStats aggregate value.
      matches,  // Supplies matches to the surrounding call or initializer.
      static_cast<float>(matches) / static_cast<float>(pixels.size()),  // Supplies static_cast<float>(matches) / static_cast<float>(pixels.size()) to the surrounding call or initializer.
  };  // Ends the current type, struct, or initializer declaration.
}  // Ends the current code block.

GearColorClassifier::WidgetColorStats GearColorClassifier::calculateWidgetColorStats(  // Implements GearColorClassifier::calculateWidgetColorStats.
    const FrameRegion& region) const {  // Starts a multi-line initializer or scope for const FrameRegion& region) const.
  const auto pixels = region.pixels();  // Sets const auto pixels to region.pixels().
  const int width = region.bounds().width;  // Sets const int width to region.bounds().width.
  const int height = region.bounds().height;  // Sets const int height to region.bounds().height.
  if (pixels.empty() || width <= 0 || height <= 0) {  // Guards the following work behind the condition pixels.empty() || width <= 0 || height <= 0.
    return {};  // Returns {} to the caller.
  }  // Ends the current code block.

  const double centerX = (static_cast<double>(width) - 1.0) / 2.0;  // Sets const double centerX to (static_cast<double>(width) - 1.0) / 2.0.
  const double centerY = (static_cast<double>(height) - 1.0) / 2.0;  // Sets const double centerY to (static_cast<double>(height) - 1.0) / 2.0.
  const double side = static_cast<double>(std::min(width, height));  // Sets const double side to static_cast<double>(std::min(width, height)).

  WidgetColorStats stats{};  // Declares stats with value initialization.
  for (int y = 0; y < height; ++y) {  // Iterates with loop control int y = 0; y < height; ++y.
    for (int x = 0; x < width; ++x) {  // Iterates with loop control int x = 0; x < width; ++x.
      const double dx = static_cast<double>(x) - centerX;  // Sets const double dx to static_cast<double>(x) - centerX.
      const double dy = static_cast<double>(y) - centerY;  // Sets const double dy to static_cast<double>(y) - centerY.
      const double radius = std::sqrt(dx * dx + dy * dy) / side;  // Sets const double radius to std::sqrt(dx * dx + dy * dy) / side.
      const auto& pixel = pixels[static_cast<std::size_t>(y * width + x)];  // Sets const auto& pixel to pixels[static_cast<std::size_t>(y * width + x)].

      if (radius >= 0.22 && radius <= 0.34) {  // Guards the following work behind the condition radius >= 0.22 && radius <= 0.34.
        ++stats.red.ringPixels;  // Increments stats.red.ringPixels by one.
        ++stats.white.ringPixels;  // Increments stats.white.ringPixels by one.
        if (thresholds_.red.matches(pixel)) {  // Guards the following work behind the condition thresholds_.red.matches(pixel).
          ++stats.red.ringCount;  // Increments stats.red.ringCount by one.
        }  // Ends the current code block.
        if (thresholds_.white.matches(pixel)) {  // Guards the following work behind the condition thresholds_.white.matches(pixel).
          ++stats.white.ringCount;  // Increments stats.white.ringCount by one.
        }  // Ends the current code block.
      } else if (radius <= 0.20) {  // Handles the alternative case where radius <= 0.20 is true.
        ++stats.red.innerPixels;  // Increments stats.red.innerPixels by one.
        ++stats.white.innerPixels;  // Increments stats.white.innerPixels by one.
        if (thresholds_.red.matches(pixel)) {  // Guards the following work behind the condition thresholds_.red.matches(pixel).
          ++stats.red.innerCount;  // Increments stats.red.innerCount by one.
        }  // Ends the current code block.
        if (thresholds_.white.matches(pixel)) {  // Guards the following work behind the condition thresholds_.white.matches(pixel).
          ++stats.white.innerCount;  // Increments stats.white.innerCount by one.
        }  // Ends the current code block.
      } else if (radius >= 0.38) {  // Handles pixels far enough from the expected red ring to be treated as background.
        ++stats.red.backgroundPixels;  // Increments stats.red.backgroundPixels by one.
        ++stats.white.backgroundPixels;  // Increments stats.white.backgroundPixels by one.
        if (thresholds_.red.matches(pixel)) {  // Guards the following work behind the condition thresholds_.red.matches(pixel).
          ++stats.red.backgroundCount;  // Increments stats.red.backgroundCount by one.
        }  // Ends the current code block.
        if (thresholds_.white.matches(pixel)) {  // Guards the following work behind the condition thresholds_.white.matches(pixel).
          ++stats.white.backgroundCount;  // Increments stats.white.backgroundCount by one.
        }  // Ends the current code block.
      }  // Ends the current code block.
    }  // Ends the current code block.
  }  // Ends the current code block.

  return stats;  // Returns stats to the caller.
}  // Ends the current code block.

bool GearColorClassifier::hasEnoughGlyphPixels(const MatchStats& stats) const {  // Implements GearColorClassifier::hasEnoughGlyphPixels.
  return stats.count >= minimumGlyphPixels_ && stats.ratio >= confidenceThreshold_;  // Returns stats.count >= minimumGlyphPixels_ && stats.ratio >= confidenceThreshold_ to the caller.
}  // Ends the current code block.

bool GearColorClassifier::hasRedGearAndRing(const WidgetMatchStats& stats, int side) const {  // Implements GearColorClassifier::hasRedGearAndRing.
  const int minimumRingPixels = std::max(24, side / 2);  // Sets const int minimumRingPixels to std::max(24, side / 2).
  const int minimumInnerPixels = std::max(24, side / 2);  // Sets const int minimumInnerPixels to std::max(24, side / 2).
  if (stats.ringCount < minimumRingPixels || stats.innerCount < minimumInnerPixels) {  // Guards the following work behind weak red ring or gear evidence.
    return false;  // Returns false to the caller.
  }  // Ends the current code block.
  if (stats.backgroundPixels <= 0) {  // Guards the following work behind the condition stats.backgroundPixels <= 0.
    return true;  // Returns true to the caller.
  }  // Ends the current code block.
  const double ringRatio = static_cast<double>(stats.ringCount) / static_cast<double>(stats.ringPixels);  // Sets const double ringRatio to the red share inside the expected ring.
  const double backgroundRatio = static_cast<double>(stats.backgroundCount) / static_cast<double>(stats.backgroundPixels);  // Sets const double backgroundRatio to the red share outside the expected gear mask.
  const bool redLooksLikeBackground = backgroundRatio > 0.18 && backgroundRatio > ringRatio * 0.45;  // Sets const bool redLooksLikeBackground to the broad-background-red rejection check.
  return !redLooksLikeBackground;  // Returns whether the red shape looks like the gear widget instead of the background.
}  // Ends the current code block.

bool GearColorClassifier::hasWhiteGearSignal(const WidgetMatchStats& stats, int side) const {  // Implements GearColorClassifier::hasWhiteGearSignal.
  const int minimumInnerPixels = std::max(24, side);  // Sets const int minimumInnerPixels to std::max(24, side).
  return stats.innerCount >= minimumInnerPixels;  // Returns stats.innerCount >= minimumInnerPixels to the caller.
}  // Ends the current code block.

}  // Ends the current code block.
