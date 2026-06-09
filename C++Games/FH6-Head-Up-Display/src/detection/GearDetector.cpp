#include "detection/GearDetector.h"

namespace fh6 {

GearDetector::GearDetector() = default;

void GearDetector::setRegion(const Rect& region) {
  calibration_.setManualRegion(region);
}

GearDetectionResult GearDetector::detectGear(const Frame& frame,
                                             const GearColorClassifier& classifier) const {
  const auto region = extractGearRegion(frame);
  if (!region || region->empty()) {
    return {};
  }

  const auto colorState = classifier.classify(*region);
  const float confidence = colorState == GearColorState::Unknown ? 0.0F : confidenceThreshold_;

  return GearDetectionResult{
      recognizeGear(*region),
      colorState,
      confidence,
      region->bounds(),
  };
}

std::optional<FrameRegion> GearDetector::extractGearRegion(const Frame& frame) const {
  const auto hudRegion = frame.crop(calibration_.getGearRegion(frame));
  if (!hudRegion || hudRegion->empty()) {
    return std::nullopt;
  }

  const auto& bounds = hudRegion->bounds();
  const Rect gearFocusRegion{
      bounds.x + static_cast<int>(static_cast<double>(bounds.width) * 0.37),
      bounds.y + static_cast<int>(static_cast<double>(bounds.height) * 0.39),
      static_cast<int>(static_cast<double>(bounds.width) * 0.37),
      static_cast<int>(static_cast<double>(bounds.height) * 0.37),
  };

  return frame.crop(gearFocusRegion);
}

GearValue GearDetector::recognizeGear(const FrameRegion& region) const {
  if (region.empty()) {
    return GearValue::Unknown;
  }

  // Real OCR/template matching belongs here once FH6 frame fixtures are available.
  return GearValue::Unknown;
}

}  // namespace fh6
