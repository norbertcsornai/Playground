#include "alert/ShiftAlertController.h"  // Imports project declarations from alert/ShiftAlertController.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

ShiftAlertController::ShiftAlertController(std::chrono::milliseconds arrowDuration)  // Begins the multi-line constructor definition for ShiftAlertController.
    : arrowDuration_(arrowDuration) {}  // Initializes constructor members with arrowDuration_(arrowDuration).

ShiftAlertState ShiftAlertController::update(const GearDetectionResult& result, TimePoint now) {  // Implements ShiftAlertController::update.
  bool triggered = false;  // Sets bool triggered to false.

  if (result.isConfident() && result.colorState == GearColorState::Red) {  // Guards the following work behind the condition result.isConfident() && result.colorState == GearColorState::Red.
    triggered = !triggeredForCurrentRed_;  // Sets triggered to !triggeredForCurrentRed_.
    triggeredForCurrentRed_ = true;  // Sets triggeredForCurrentRed_ to true.
    alertActiveUntil_ = now + arrowDuration_;  // Sets alertActiveUntil_ to now + arrowDuration_.
  } else if (result.isConfident() && result.colorState == GearColorState::White) {  // Handles the alternative case where result.isConfident() && result.colorState == GearColorState::White is true.
    alertActiveUntil_ = now;  // Sets alertActiveUntil_ to now.
  }  // Ends the current code block.

  if (result.colorState != GearColorState::Unknown && result.isConfident()) {  // Guards the following work behind the condition result.colorState != GearColorState::Unknown && result.isConfident().
    if (result.colorState == GearColorState::White) {  // Guards the following work behind the condition result.colorState == GearColorState::White.
      triggeredForCurrentRed_ = false;  // Sets triggeredForCurrentRed_ to false.
    }  // Ends the current code block.
    previousColorState_ = result.colorState;  // Sets previousColorState_ to result.colorState.
  }  // Ends the current code block.

  return ShiftAlertState{  // Starts returning a ShiftAlertState aggregate value.
      isAlertActive(now),  // Supplies isAlertActive(now) to the surrounding call or initializer.
      triggered,  // Supplies triggered to the surrounding call or initializer.
      alertActiveUntil_,  // Supplies alertActiveUntil_ to the surrounding call or initializer.
  };  // Ends the current type, struct, or initializer declaration.
}  // Ends the current code block.

void ShiftAlertController::reset() {  // Implements ShiftAlertController::reset.
  previousColorState_ = GearColorState::Unknown;  // Sets previousColorState_ to GearColorState::Unknown.
  alertActiveUntil_ = {};  // Sets alertActiveUntil_ to {}.
  triggeredForCurrentRed_ = false;  // Sets triggeredForCurrentRed_ to false.
}  // Ends the current code block.

bool ShiftAlertController::isAlertActive(TimePoint now) const {  // Implements ShiftAlertController::isAlertActive.
  return alertActiveUntil_ > now;  // Returns alertActiveUntil_ > now to the caller.
}  // Ends the current code block.

}  // Ends the current code block.
