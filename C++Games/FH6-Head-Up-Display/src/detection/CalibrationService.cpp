#include "detection/CalibrationService.h"

#include <algorithm>

namespace fh6 {

CalibrationService::CalibrationService()
    : defaultRegion_(Rect{-1, -1, 420, 420}), currentRegion_(defaultRegion_) {}

std::optional<Rect> CalibrationService::autoLocateGearRegion(const Frame& frame) const {
  if (frame.empty()) {
    return std::nullopt;
  }

  return getGearRegion(frame);
}

void CalibrationService::setManualRegion(const Rect& region) {
  if (!region.empty()) {
    currentRegion_ = region;
  }
}

Rect CalibrationService::getGearRegion() const {
  return currentRegion_;
}

Rect CalibrationService::getGearRegion(const Frame& frame) const {
  if (currentRegion_.x < 0 || currentRegion_.y < 0) {
    return resolveDynamicRegion(frame);
  }

  return currentRegion_;
}

void CalibrationService::resetToDefaults() {
  currentRegion_ = defaultRegion_;
}

Rect CalibrationService::resolveDynamicRegion(const Frame& frame) const {
  const int size = std::max(220, static_cast<int>(static_cast<double>(frame.height()) * 0.373));
  const int width = size;
  const int height = size;
  const int marginX = std::max(12, static_cast<int>(static_cast<double>(frame.width()) * 0.010));
  const int marginY = std::max(12, static_cast<int>(static_cast<double>(frame.height()) * 0.018));

  return Rect{
      frame.width() - width - marginX,
      frame.height() - height - marginY,
      width,
      height,
  };
}

}  // namespace fh6
