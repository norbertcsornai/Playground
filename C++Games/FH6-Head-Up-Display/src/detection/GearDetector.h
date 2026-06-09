#pragma once

#include "capture/Frame.h"
#include "detection/CalibrationService.h"
#include "detection/GearColorClassifier.h"
#include "detection/GearDetectionResult.h"
#include "shared/Geometry.h"

namespace fh6 {

class GearDetector {
 public:
  GearDetector();

  void setRegion(const Rect& region);
  [[nodiscard]] GearDetectionResult detectGear(const Frame& frame,
                                               const GearColorClassifier& classifier) const;

 private:
  [[nodiscard]] std::optional<FrameRegion> extractGearRegion(const Frame& frame) const;
  [[nodiscard]] GearValue recognizeGear(const FrameRegion& region) const;

  CalibrationService calibration_{};
  float confidenceThreshold_{0.65F};
};

}  // namespace fh6
