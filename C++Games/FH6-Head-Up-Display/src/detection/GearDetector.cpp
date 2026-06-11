#include "detection/GearDetector.h"  // Imports project declarations from detection/GearDetector.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

GearDetector::GearDetector() = default;  // Sets GearDetector::GearDetector() to default.

void GearDetector::setRegion(const Rect& region) {  // Implements GearDetector::setRegion.
  calibration_.setManualRegion(region);  // Calls setManualRegion on calibration_.
}  // Ends the current code block.

GearDetectionResult GearDetector::detectGear(const Frame& frame,  // Implements GearDetector::detectGear.
                                             const GearColorClassifier& classifier) const {  // Starts a multi-line initializer or scope for const GearColorClassifier& classifier) const.
  const auto region = extractGearRegion(frame);  // Sets const auto region to extractGearRegion(frame).
  if (!region || region->empty()) {  // Guards the following work behind the condition !region || region->empty().
    return {};  // Returns {} to the caller.
  }  // Ends the current code block.

  const auto colorState = classifier.classify(*region);  // Sets const auto colorState to classifier.classify(*region).
  const float confidence = colorState == GearColorState::Unknown ? 0.0F : confidenceThreshold_;  // Sets const float confidence to colorState == GearColorState::Unknown ? 0.0F : confidenceThreshold_.

  return GearDetectionResult{  // Starts returning a GearDetectionResult aggregate value.
      recognizeGear(*region),  // Supplies recognizeGear(*region) to the surrounding call or initializer.
      colorState,  // Supplies colorState to the surrounding call or initializer.
      confidence,  // Supplies confidence to the surrounding call or initializer.
      region->bounds(),  // Supplies region->bounds() to the surrounding call or initializer.
  };  // Ends the current type, struct, or initializer declaration.
}  // Ends the current code block.

std::optional<FrameRegion> GearDetector::extractGearRegion(const Frame& frame) const {  // Implements GearDetector::extractGearRegion.
  const auto hudBounds = frame.clippedBounds(calibration_.getGearRegion(frame));  // Sets const auto hudBounds to frame.clippedBounds(calibration_.getGearRegion(frame)).
  if (!hudBounds) {  // Guards the following work behind the condition !hudBounds.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  const auto& bounds = *hudBounds;  // Sets const auto& bounds to *hudBounds.
  const Rect gearFocusRegion{  // Starts a multi-line initializer or scope for const Rect gearFocusRegion.
      bounds.x + static_cast<int>(static_cast<double>(bounds.width) * 0.37),  // Supplies bounds.x + static_cast<int>(static_cast<double>(bounds.width) * 0.37) to the surrounding call or initializer.
      bounds.y + static_cast<int>(static_cast<double>(bounds.height) * 0.39),  // Supplies bounds.y + static_cast<int>(static_cast<double>(bounds.height) * 0.39) to the surrounding call or initializer.
      static_cast<int>(static_cast<double>(bounds.width) * 0.37),  // Supplies static_cast<int>(static_cast<double>(bounds.width) * 0.37) to the surrounding call or initializer.
      static_cast<int>(static_cast<double>(bounds.height) * 0.37),  // Supplies static_cast<int>(static_cast<double>(bounds.height) * 0.37) to the surrounding call or initializer.
  };  // Ends the current type, struct, or initializer declaration.

  return frame.crop(gearFocusRegion);  // Returns frame.crop(gearFocusRegion) to the caller.
}  // Ends the current code block.

GearValue GearDetector::recognizeGear(const FrameRegion& region) const {  // Implements GearDetector::recognizeGear.
  if (region.empty()) {  // Guards the following work behind the condition region.empty().
    return GearValue::Unknown;  // Returns GearValue::Unknown to the caller.
  }  // Ends the current code block.


  return GearValue::Unknown;  // Returns GearValue::Unknown to the caller.
}  // Ends the current code block.

}  // Ends the current code block.
