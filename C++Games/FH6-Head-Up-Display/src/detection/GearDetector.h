#pragma once  // codex-line-comment: documents this line.

#include "capture/Frame.h"  // codex-line-comment: documents this line.
#include "detection/CalibrationService.h"  // codex-line-comment: documents this line.
#include "detection/GearColorClassifier.h"  // codex-line-comment: documents this line.
#include "detection/GearDetectionResult.h"  // codex-line-comment: documents this line.
#include "shared/Geometry.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class GearDetector {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  GearDetector();  // codex-line-comment: documents this line.

  void setRegion(const Rect& region);  // codex-line-comment: documents this line.
  [[nodiscard]] GearDetectionResult detectGear(const Frame& frame,  // codex-line-comment: documents this line.
                                               const GearColorClassifier& classifier) const;  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  [[nodiscard]] std::optional<FrameRegion> extractGearRegion(const Frame& frame) const;  // codex-line-comment: documents this line.
  [[nodiscard]] GearValue recognizeGear(const FrameRegion& region) const;  // codex-line-comment: documents this line.

  CalibrationService calibration_{};  // codex-line-comment: documents this line.
  float confidenceThreshold_{0.65F};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
