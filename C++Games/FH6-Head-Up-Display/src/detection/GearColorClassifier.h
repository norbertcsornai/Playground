#pragma once

#include "capture/Frame.h"
#include "shared/Color.h"
#include "shared/GearTypes.h"

namespace fh6 {

class GearColorClassifier {
 public:
  GearColorClassifier() = default;
  explicit GearColorClassifier(ColorThresholds thresholds);

  [[nodiscard]] GearColorState classify(const FrameRegion& region) const;
  void updateThresholds(const ColorThresholds& thresholds);

 private:
  struct MatchStats {
    int count{0};
    float ratio{0.0F};
  };

  struct WidgetMatchStats {
    int ringCount{0};
    int innerCount{0};
    int ringPixels{0};
    int innerPixels{0};
  };

  [[nodiscard]] MatchStats calculateMatchStats(const FrameRegion& region,
                                               const ColorThreshold& target) const;
  [[nodiscard]] WidgetMatchStats calculateWidgetMatchStats(const FrameRegion& region,
                                                           const ColorThreshold& target) const;
  [[nodiscard]] bool hasEnoughGlyphPixels(const MatchStats& stats) const;
  [[nodiscard]] bool hasRedGearAndRing(const WidgetMatchStats& stats, int side) const;
  [[nodiscard]] bool hasWhiteGearSignal(const WidgetMatchStats& stats, int side) const;

  ColorThresholds thresholds_{};
  float confidenceThreshold_{0.001F};
  int minimumGlyphPixels_{12};
};

}  // namespace fh6
