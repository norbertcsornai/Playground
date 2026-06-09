#pragma once

#include "shared/GearTypes.h"
#include "shared/Geometry.h"

namespace fh6 {

struct GearDetectionResult {
  GearValue value{GearValue::Unknown};
  GearColorState colorState{GearColorState::Unknown};
  float confidence{0.0F};
  Rect region{};

  [[nodiscard]] bool isConfident() const {
    return confidence >= 0.65F && !region.empty();
  }
};

}  // namespace fh6
