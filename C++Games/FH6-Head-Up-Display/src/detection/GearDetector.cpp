#include "detection/GearDetector.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

GearDetector::GearDetector() = default;  // codex-line-comment: documents this line.

void GearDetector::setRegion(const Rect& region) {  // codex-line-comment: documents this line.
  calibration_.setManualRegion(region);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

GearDetectionResult GearDetector::detectGear(const Frame& frame,  // codex-line-comment: documents this line.
                                             const GearColorClassifier& classifier) const {  // codex-line-comment: documents this line.
  const auto region = extractGearRegion(frame);  // codex-line-comment: documents this line.
  if (!region || region->empty()) {  // codex-line-comment: documents this line.
    return {};  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  const auto colorState = classifier.classify(*region);  // codex-line-comment: documents this line.
  const float confidence = colorState == GearColorState::Unknown ? 0.0F : confidenceThreshold_;  // codex-line-comment: documents this line.

  return GearDetectionResult{  // codex-line-comment: documents this line.
      recognizeGear(*region),  // codex-line-comment: documents this line.
      colorState,  // codex-line-comment: documents this line.
      confidence,  // codex-line-comment: documents this line.
      region->bounds(),  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

std::optional<FrameRegion> GearDetector::extractGearRegion(const Frame& frame) const {  // codex-line-comment: documents this line.
  const auto hudRegion = frame.crop(calibration_.getGearRegion(frame));  // codex-line-comment: documents this line.
  if (!hudRegion || hudRegion->empty()) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  const auto& bounds = hudRegion->bounds();  // codex-line-comment: documents this line.
  const Rect gearFocusRegion{  // codex-line-comment: documents this line.
      bounds.x + static_cast<int>(static_cast<double>(bounds.width) * 0.37),  // codex-line-comment: documents this line.
      bounds.y + static_cast<int>(static_cast<double>(bounds.height) * 0.39),  // codex-line-comment: documents this line.
      static_cast<int>(static_cast<double>(bounds.width) * 0.37),  // codex-line-comment: documents this line.
      static_cast<int>(static_cast<double>(bounds.height) * 0.37),  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.

  return frame.crop(gearFocusRegion);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

GearValue GearDetector::recognizeGear(const FrameRegion& region) const {  // codex-line-comment: documents this line.
  if (region.empty()) {  // codex-line-comment: documents this line.
    return GearValue::Unknown;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  // Real OCR/template matching belongs here once FH6 frame fixtures are available.  // codex-line-comment: documents this line.
  return GearValue::Unknown;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
