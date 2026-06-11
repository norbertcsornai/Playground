#pragma once  // Prevents this header from being included more than once.

#include "capture/Frame.h"  // Imports project declarations from capture/Frame.h.
#include "detection/CalibrationService.h"  // Imports project declarations from detection/CalibrationService.h.
#include "detection/GearColorClassifier.h"  // Imports project declarations from detection/GearColorClassifier.h.
#include "detection/GearDetectionResult.h"  // Imports project declarations from detection/GearDetectionResult.h.
#include "shared/Geometry.h"  // Imports project declarations from shared/Geometry.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class GearDetector {  // Declares the GearDetector class interface and members.
 public:  // Makes the following members part of the public API.
  GearDetector();  // Invokes GearDetector with the supplied arguments.

  void setRegion(const Rect& region);  // Declares function setRegion for callers.
  [[nodiscard]] GearDetectionResult detectGear(const Frame& frame,  // Supplies [[nodiscard]] GearDetectionResult detectGear(const Frame& frame to the surrounding call or initializer.
                                               const GearColorClassifier& classifier) const;  // Declares const for use in this scope.

 private:  // Makes the following members private implementation details.
  [[nodiscard]] std::optional<FrameRegion> extractGearRegion(const Frame& frame) const;  // Declares extractGearRegion and marks its return value as important.
  [[nodiscard]] GearValue recognizeGear(const FrameRegion& region) const;  // Declares recognizeGear and marks its return value as important.

  CalibrationService calibration_{};  // Declares calibration_ with value initialization.
  float confidenceThreshold_{0.65F};  // Declares confidenceThreshold_ and initializes it with 0.65F.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
