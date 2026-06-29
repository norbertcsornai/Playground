#pragma once  // Prevents this header from being included more than once.

#include <chrono>  // Imports the chrono standard library declarations used in this file.

#include "alert/ShiftAlertState.h"  // Imports project declarations from alert/ShiftAlertState.h.
#include "detection/GearDetectionResult.h"  // Imports project declarations from detection/GearDetectionResult.h.
#include "shared/GearTypes.h"  // Imports project declarations from shared/GearTypes.h.
#include "shared/Time.h"  // Imports project declarations from shared/Time.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class ShiftAlertController {  // Declares the ShiftAlertController class interface and members.
 public:  // Makes the following members part of the public API.
  explicit ShiftAlertController(std::chrono::milliseconds arrowDuration = std::chrono::milliseconds(1500));  // Declares the explicit ShiftAlertController constructor.

  [[nodiscard]] ShiftAlertState update(const GearDetectionResult& result, TimePoint now);  // Declares update and marks its return value as important.
  void reset();  // Declares function reset for callers.
  [[nodiscard]] bool isAlertActive(TimePoint now) const;  // Declares isAlertActive and marks its return value as important.

 private:  // Makes the following members private implementation details.
  GearColorState previousColorState_{GearColorState::Unknown};  // Declares previousColorState_ and initializes it with GearColorState::Unknown.
  TimePoint alertActiveUntil_{};  // Declares alertActiveUntil_ with value initialization.
  std::chrono::milliseconds arrowDuration_{1500};  // Declares arrowDuration_ and initializes it with 1500.
  bool armedByWhite_{false};  // Tracks whether a confident white gear frame has armed the next red trigger.
  bool triggeredForCurrentRed_{false};  // Declares triggeredForCurrentRed_ and initializes it with false.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
