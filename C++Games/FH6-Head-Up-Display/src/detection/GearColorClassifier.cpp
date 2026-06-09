#include "detection/GearColorClassifier.h"

#include <algorithm>
#include <cmath>

namespace fh6 {

GearColorClassifier::GearColorClassifier(ColorThresholds thresholds) : thresholds_(thresholds) {}

GearColorState GearColorClassifier::classify(const FrameRegion& region) const {
  if (region.empty()) {
    return GearColorState::Unknown;
  }

  const int side = std::min(region.bounds().width, region.bounds().height);
  const auto redWidgetStats = calculateWidgetMatchStats(region, thresholds_.red);
  const auto whiteWidgetStats = calculateWidgetMatchStats(region, thresholds_.white);

  if (hasRedGearAndRing(redWidgetStats, side)) {
    return GearColorState::Red;
  }

  if (hasWhiteGearSignal(whiteWidgetStats, side)) {
    return GearColorState::White;
  }

  const auto whiteStats = calculateMatchStats(region, thresholds_.white);

  if (hasEnoughGlyphPixels(whiteStats)) {
    return GearColorState::White;
  }

  return GearColorState::Unknown;
}

void GearColorClassifier::updateThresholds(const ColorThresholds& thresholds) {
  thresholds_ = thresholds;
}

GearColorClassifier::MatchStats GearColorClassifier::calculateMatchStats(
    const FrameRegion& region, const ColorThreshold& target) const {
  const auto pixels = region.pixels();
  if (pixels.empty()) {
    return {};
  }

  int matches = 0;
  for (const auto& pixel : pixels) {
    if (target.matches(pixel)) {
      ++matches;
    }
  }

  return MatchStats{
      matches,
      static_cast<float>(matches) / static_cast<float>(pixels.size()),
  };
}

GearColorClassifier::WidgetMatchStats GearColorClassifier::calculateWidgetMatchStats(
    const FrameRegion& region, const ColorThreshold& target) const {
  const auto pixels = region.pixels();
  const int width = region.bounds().width;
  const int height = region.bounds().height;
  if (pixels.empty() || width <= 0 || height <= 0) {
    return {};
  }

  const double centerX = (static_cast<double>(width) - 1.0) / 2.0;
  const double centerY = (static_cast<double>(height) - 1.0) / 2.0;
  const double side = static_cast<double>(std::min(width, height));

  WidgetMatchStats stats{};
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const double dx = static_cast<double>(x) - centerX;
      const double dy = static_cast<double>(y) - centerY;
      const double radius = std::sqrt(dx * dx + dy * dy) / side;
      const auto& pixel = pixels[static_cast<std::size_t>(y * width + x)];

      if (radius >= 0.22 && radius <= 0.34) {
        ++stats.ringPixels;
        if (target.matches(pixel)) {
          ++stats.ringCount;
        }
      } else if (radius <= 0.20) {
        ++stats.innerPixels;
        if (target.matches(pixel)) {
          ++stats.innerCount;
        }
      }
    }
  }

  return stats;
}

bool GearColorClassifier::hasEnoughGlyphPixels(const MatchStats& stats) const {
  return stats.count >= minimumGlyphPixels_ && stats.ratio >= confidenceThreshold_;
}

bool GearColorClassifier::hasRedGearAndRing(const WidgetMatchStats& stats, int side) const {
  const int minimumRingPixels = std::max(24, side / 2);
  const int minimumInnerPixels = std::max(24, side / 2);
  return stats.ringCount >= minimumRingPixels && stats.innerCount >= minimumInnerPixels;
}

bool GearColorClassifier::hasWhiteGearSignal(const WidgetMatchStats& stats, int side) const {
  const int minimumInnerPixels = std::max(24, side);
  return stats.innerCount >= minimumInnerPixels;
}

}  // namespace fh6
