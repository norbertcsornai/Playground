#pragma once  // codex-line-comment: documents this line.

#include "capture/Frame.h"  // codex-line-comment: documents this line.
#include "shared/Color.h"  // codex-line-comment: documents this line.
#include "shared/GearTypes.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class GearColorClassifier {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  GearColorClassifier() = default;  // codex-line-comment: documents this line.
  explicit GearColorClassifier(ColorThresholds thresholds);  // codex-line-comment: documents this line.

  [[nodiscard]] GearColorState classify(const FrameRegion& region) const;  // codex-line-comment: documents this line.
  void updateThresholds(const ColorThresholds& thresholds);  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  struct MatchStats {  // codex-line-comment: documents this line.
    int count{0};  // codex-line-comment: documents this line.
    float ratio{0.0F};  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.

  struct WidgetMatchStats {  // codex-line-comment: documents this line.
    int ringCount{0};  // codex-line-comment: documents this line.
    int innerCount{0};  // codex-line-comment: documents this line.
    int ringPixels{0};  // codex-line-comment: documents this line.
    int innerPixels{0};  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.

  [[nodiscard]] MatchStats calculateMatchStats(const FrameRegion& region,  // codex-line-comment: documents this line.
                                               const ColorThreshold& target) const;  // codex-line-comment: documents this line.
  [[nodiscard]] WidgetMatchStats calculateWidgetMatchStats(const FrameRegion& region,  // codex-line-comment: documents this line.
                                                           const ColorThreshold& target) const;  // codex-line-comment: documents this line.
  [[nodiscard]] bool hasEnoughGlyphPixels(const MatchStats& stats) const;  // codex-line-comment: documents this line.
  [[nodiscard]] bool hasRedGearAndRing(const WidgetMatchStats& stats, int side) const;  // codex-line-comment: documents this line.
  [[nodiscard]] bool hasWhiteGearSignal(const WidgetMatchStats& stats, int side) const;  // codex-line-comment: documents this line.

  ColorThresholds thresholds_{};  // codex-line-comment: documents this line.
  float confidenceThreshold_{0.001F};  // codex-line-comment: documents this line.
  int minimumGlyphPixels_{12};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
