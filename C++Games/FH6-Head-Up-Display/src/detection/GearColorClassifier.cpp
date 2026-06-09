#include "detection/GearColorClassifier.h"  // codex-line-comment: documents this line.

#include <algorithm>  // codex-line-comment: documents this line.
#include <cmath>  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

GearColorClassifier::GearColorClassifier(ColorThresholds thresholds) : thresholds_(thresholds) {}  // codex-line-comment: documents this line.

GearColorState GearColorClassifier::classify(const FrameRegion& region) const {  // codex-line-comment: documents this line.
  if (region.empty()) {  // codex-line-comment: documents this line.
    return GearColorState::Unknown;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  const int side = std::min(region.bounds().width, region.bounds().height);  // codex-line-comment: documents this line.
  const auto redWidgetStats = calculateWidgetMatchStats(region, thresholds_.red);  // codex-line-comment: documents this line.
  const auto whiteWidgetStats = calculateWidgetMatchStats(region, thresholds_.white);  // codex-line-comment: documents this line.

  if (hasRedGearAndRing(redWidgetStats, side)) {  // codex-line-comment: documents this line.
    return GearColorState::Red;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (hasWhiteGearSignal(whiteWidgetStats, side)) {  // codex-line-comment: documents this line.
    return GearColorState::White;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  const auto whiteStats = calculateMatchStats(region, thresholds_.white);  // codex-line-comment: documents this line.

  if (hasEnoughGlyphPixels(whiteStats)) {  // codex-line-comment: documents this line.
    return GearColorState::White;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return GearColorState::Unknown;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void GearColorClassifier::updateThresholds(const ColorThresholds& thresholds) {  // codex-line-comment: documents this line.
  thresholds_ = thresholds;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

GearColorClassifier::MatchStats GearColorClassifier::calculateMatchStats(  // codex-line-comment: documents this line.
    const FrameRegion& region, const ColorThreshold& target) const {  // codex-line-comment: documents this line.
  const auto pixels = region.pixels();  // codex-line-comment: documents this line.
  if (pixels.empty()) {  // codex-line-comment: documents this line.
    return {};  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  int matches = 0;  // codex-line-comment: documents this line.
  for (const auto& pixel : pixels) {  // codex-line-comment: documents this line.
    if (target.matches(pixel)) {  // codex-line-comment: documents this line.
      ++matches;  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return MatchStats{  // codex-line-comment: documents this line.
      matches,  // codex-line-comment: documents this line.
      static_cast<float>(matches) / static_cast<float>(pixels.size()),  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

GearColorClassifier::WidgetMatchStats GearColorClassifier::calculateWidgetMatchStats(  // codex-line-comment: documents this line.
    const FrameRegion& region, const ColorThreshold& target) const {  // codex-line-comment: documents this line.
  const auto pixels = region.pixels();  // codex-line-comment: documents this line.
  const int width = region.bounds().width;  // codex-line-comment: documents this line.
  const int height = region.bounds().height;  // codex-line-comment: documents this line.
  if (pixels.empty() || width <= 0 || height <= 0) {  // codex-line-comment: documents this line.
    return {};  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  const double centerX = (static_cast<double>(width) - 1.0) / 2.0;  // codex-line-comment: documents this line.
  const double centerY = (static_cast<double>(height) - 1.0) / 2.0;  // codex-line-comment: documents this line.
  const double side = static_cast<double>(std::min(width, height));  // codex-line-comment: documents this line.

  WidgetMatchStats stats{};  // codex-line-comment: documents this line.
  for (int y = 0; y < height; ++y) {  // codex-line-comment: documents this line.
    for (int x = 0; x < width; ++x) {  // codex-line-comment: documents this line.
      const double dx = static_cast<double>(x) - centerX;  // codex-line-comment: documents this line.
      const double dy = static_cast<double>(y) - centerY;  // codex-line-comment: documents this line.
      const double radius = std::sqrt(dx * dx + dy * dy) / side;  // codex-line-comment: documents this line.
      const auto& pixel = pixels[static_cast<std::size_t>(y * width + x)];  // codex-line-comment: documents this line.

      if (radius >= 0.22 && radius <= 0.34) {  // codex-line-comment: documents this line.
        ++stats.ringPixels;  // codex-line-comment: documents this line.
        if (target.matches(pixel)) {  // codex-line-comment: documents this line.
          ++stats.ringCount;  // codex-line-comment: documents this line.
        }  // codex-line-comment: documents this line.
      } else if (radius <= 0.20) {  // codex-line-comment: documents this line.
        ++stats.innerPixels;  // codex-line-comment: documents this line.
        if (target.matches(pixel)) {  // codex-line-comment: documents this line.
          ++stats.innerCount;  // codex-line-comment: documents this line.
        }  // codex-line-comment: documents this line.
      }  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return stats;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool GearColorClassifier::hasEnoughGlyphPixels(const MatchStats& stats) const {  // codex-line-comment: documents this line.
  return stats.count >= minimumGlyphPixels_ && stats.ratio >= confidenceThreshold_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool GearColorClassifier::hasRedGearAndRing(const WidgetMatchStats& stats, int side) const {  // codex-line-comment: documents this line.
  const int minimumRingPixels = std::max(24, side / 2);  // codex-line-comment: documents this line.
  const int minimumInnerPixels = std::max(24, side / 2);  // codex-line-comment: documents this line.
  return stats.ringCount >= minimumRingPixels && stats.innerCount >= minimumInnerPixels;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool GearColorClassifier::hasWhiteGearSignal(const WidgetMatchStats& stats, int side) const {  // codex-line-comment: documents this line.
  const int minimumInnerPixels = std::max(24, side);  // codex-line-comment: documents this line.
  return stats.innerCount >= minimumInnerPixels;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
