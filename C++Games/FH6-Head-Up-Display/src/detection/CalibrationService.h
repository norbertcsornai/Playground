#pragma once

#include <optional>

#include "capture/Frame.h"
#include "shared/Geometry.h"

namespace fh6 {

class CalibrationService {
 public:
  CalibrationService();

  [[nodiscard]] std::optional<Rect> autoLocateGearRegion(const Frame& frame) const;
  void setManualRegion(const Rect& region);
  [[nodiscard]] Rect getGearRegion() const;
  [[nodiscard]] Rect getGearRegion(const Frame& frame) const;
  void resetToDefaults();

 private:
  [[nodiscard]] Rect resolveDynamicRegion(const Frame& frame) const;

  Rect defaultRegion_{};
  Rect currentRegion_{};
};

}  // namespace fh6
