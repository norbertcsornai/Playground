#include "detection/CalibrationService.h"  // Imports project declarations from detection/CalibrationService.h.

#include <algorithm>  // Imports the algorithm standard library declarations used in this file.

namespace fh6 {  // Places the following declarations inside namespace fh6.

CalibrationService::CalibrationService()  // Begins the multi-line constructor definition for CalibrationService.
    : defaultRegion_(Rect{-1, -1, 420, 420}), currentRegion_(defaultRegion_) {}  // Initializes constructor members with defaultRegion_(Rect{-1, -1, 420, 420}), currentRegion_(defaultRegion_).

std::optional<Rect> CalibrationService::autoLocateGearRegion(const Frame& frame) const {  // Implements CalibrationService::autoLocateGearRegion.
  if (frame.empty()) {  // Guards the following work behind the condition frame.empty().
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  return getGearRegion(frame);  // Returns getGearRegion(frame) to the caller.
}  // Ends the current code block.

void CalibrationService::setManualRegion(const Rect& region) {  // Implements CalibrationService::setManualRegion.
  if (!region.empty()) {  // Guards the following work behind the condition !region.empty().
    currentRegion_ = region;  // Sets currentRegion_ to region.
  }  // Ends the current code block.
}  // Ends the current code block.

Rect CalibrationService::getGearRegion() const {  // Implements CalibrationService::getGearRegion.
  return currentRegion_;  // Returns currentRegion_ to the caller.
}  // Ends the current code block.

Rect CalibrationService::getGearRegion(const Frame& frame) const {  // Implements CalibrationService::getGearRegion.
  if (currentRegion_.x < 0 || currentRegion_.y < 0) {  // Guards the following work behind the condition currentRegion_.x < 0 || currentRegion_.y < 0.
    return resolveDynamicRegion(frame);  // Returns resolveDynamicRegion(frame) to the caller.
  }  // Ends the current code block.

  return currentRegion_;  // Returns currentRegion_ to the caller.
}  // Ends the current code block.

void CalibrationService::resetToDefaults() {  // Implements CalibrationService::resetToDefaults.
  currentRegion_ = defaultRegion_;  // Sets currentRegion_ to defaultRegion_.
}  // Ends the current code block.

Rect CalibrationService::resolveDynamicRegion(const Frame& frame) const {  // Implements CalibrationService::resolveDynamicRegion.
  const int size = std::max(220, static_cast<int>(static_cast<double>(frame.height()) * 0.373));  // Sets const int size to std::max(220, static_cast<int>(static_cast<double>(frame.height()) * 0.373)).
  const int width = size;  // Sets const int width to size.
  const int height = size;  // Sets const int height to size.
  const int marginX = std::max(12, static_cast<int>(static_cast<double>(frame.width()) * 0.010));  // Sets const int marginX to std::max(12, static_cast<int>(static_cast<double>(frame.width()) * 0.010)).
  const int marginY = std::max(12, static_cast<int>(static_cast<double>(frame.height()) * 0.018));  // Sets const int marginY to std::max(12, static_cast<int>(static_cast<double>(frame.height()) * 0.018)).

  return Rect{  // Starts returning a Rect aggregate value.
      frame.width() - width - marginX,  // Supplies frame.width() - width - marginX to the surrounding call or initializer.
      frame.height() - height - marginY,  // Supplies frame.height() - height - marginY to the surrounding call or initializer.
      width,  // Supplies width to the surrounding call or initializer.
      height,  // Supplies height to the surrounding call or initializer.
  };  // Ends the current type, struct, or initializer declaration.
}  // Ends the current code block.

}  // Ends the current code block.
