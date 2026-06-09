#pragma once  // codex-line-comment: documents this line.

#include "shared/GearTypes.h"  // codex-line-comment: documents this line.
#include "shared/Geometry.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

struct GearDetectionResult {  // codex-line-comment: documents this line.
  GearValue value{GearValue::Unknown};  // codex-line-comment: documents this line.
  GearColorState colorState{GearColorState::Unknown};  // codex-line-comment: documents this line.
  float confidence{0.0F};  // codex-line-comment: documents this line.
  Rect region{};  // codex-line-comment: documents this line.

  [[nodiscard]] bool isConfident() const {  // codex-line-comment: documents this line.
    return confidence >= 0.65F && !region.empty();  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
