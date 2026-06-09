#include "detection/CalibrationService.h"  // codex-line-comment: documents this line.

#include <algorithm>  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

CalibrationService::CalibrationService()  // codex-line-comment: documents this line.
    : defaultRegion_(Rect{-1, -1, 420, 420}), currentRegion_(defaultRegion_) {}  // codex-line-comment: documents this line.

std::optional<Rect> CalibrationService::autoLocateGearRegion(const Frame& frame) const {  // codex-line-comment: documents this line.
  if (frame.empty()) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return getGearRegion(frame);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void CalibrationService::setManualRegion(const Rect& region) {  // codex-line-comment: documents this line.
  if (!region.empty()) {  // codex-line-comment: documents this line.
    currentRegion_ = region;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

Rect CalibrationService::getGearRegion() const {  // codex-line-comment: documents this line.
  return currentRegion_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

Rect CalibrationService::getGearRegion(const Frame& frame) const {  // codex-line-comment: documents this line.
  if (currentRegion_.x < 0 || currentRegion_.y < 0) {  // codex-line-comment: documents this line.
    return resolveDynamicRegion(frame);  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return currentRegion_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void CalibrationService::resetToDefaults() {  // codex-line-comment: documents this line.
  currentRegion_ = defaultRegion_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

Rect CalibrationService::resolveDynamicRegion(const Frame& frame) const {  // codex-line-comment: documents this line.
  const int size = std::max(220, static_cast<int>(static_cast<double>(frame.height()) * 0.373));  // codex-line-comment: documents this line.
  const int width = size;  // codex-line-comment: documents this line.
  const int height = size;  // codex-line-comment: documents this line.
  const int marginX = std::max(12, static_cast<int>(static_cast<double>(frame.width()) * 0.010));  // codex-line-comment: documents this line.
  const int marginY = std::max(12, static_cast<int>(static_cast<double>(frame.height()) * 0.018));  // codex-line-comment: documents this line.

  return Rect{  // codex-line-comment: documents this line.
      frame.width() - width - marginX,  // codex-line-comment: documents this line.
      frame.height() - height - marginY,  // codex-line-comment: documents this line.
      width,  // codex-line-comment: documents this line.
      height,  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
