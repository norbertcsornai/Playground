#pragma once  // Prevents this header from being included more than once.

#include "shared/GearTypes.h"  // Imports project declarations from shared/GearTypes.h.
#include "shared/Geometry.h"  // Imports project declarations from shared/Geometry.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

struct GearDetectionResult {  // Declares the GearDetectionResult value type and fields.
  GearValue value{GearValue::Unknown};  // Declares value and initializes it with GearValue::Unknown.
  GearColorState colorState{GearColorState::Unknown};  // Declares colorState and initializes it with GearColorState::Unknown.
  float confidence{0.0F};  // Declares confidence and initializes it with 0.0F.
  Rect region{};  // Declares region with value initialization.

  [[nodiscard]] bool isConfident() const {  // Begins function isConfident.
    return confidence >= 0.65F && !region.empty();  // Returns confidence >= 0.65F && !region.empty() to the caller.
  }  // Ends the current code block.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
