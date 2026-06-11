#pragma once  // Prevents this header from being included more than once.

#include "shared/Time.h"  // Imports project declarations from shared/Time.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

struct ShiftAlertState {  // Declares the ShiftAlertState value type and fields.
  bool active{false};  // Declares active and initializes it with false.
  bool triggeredThisFrame{false};  // Declares triggeredThisFrame and initializes it with false.
  TimePoint expiresAt{};  // Declares expiresAt with value initialization.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
