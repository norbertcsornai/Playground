#include "alert/ShiftAlertController.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

ShiftAlertController::ShiftAlertController(std::chrono::milliseconds arrowDuration)  // codex-line-comment: documents this line.
    : arrowDuration_(arrowDuration) {}  // codex-line-comment: documents this line.

ShiftAlertState ShiftAlertController::update(const GearDetectionResult& result, TimePoint now) {  // codex-line-comment: documents this line.
  bool triggered = false;  // codex-line-comment: documents this line.

  if (result.isConfident() && result.colorState == GearColorState::Red) {  // codex-line-comment: documents this line.
    triggered = !triggeredForCurrentRed_;  // codex-line-comment: documents this line.
    triggeredForCurrentRed_ = true;  // codex-line-comment: documents this line.
    alertActiveUntil_ = now + arrowDuration_;  // codex-line-comment: documents this line.
  } else if (result.isConfident() && result.colorState == GearColorState::White) {  // codex-line-comment: documents this line.
    alertActiveUntil_ = now;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (result.colorState != GearColorState::Unknown && result.isConfident()) {  // codex-line-comment: documents this line.
    if (result.colorState == GearColorState::White) {  // codex-line-comment: documents this line.
      triggeredForCurrentRed_ = false;  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
    previousColorState_ = result.colorState;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return ShiftAlertState{  // codex-line-comment: documents this line.
      isAlertActive(now),  // codex-line-comment: documents this line.
      triggered,  // codex-line-comment: documents this line.
      alertActiveUntil_,  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void ShiftAlertController::reset() {  // codex-line-comment: documents this line.
  previousColorState_ = GearColorState::Unknown;  // codex-line-comment: documents this line.
  alertActiveUntil_ = {};  // codex-line-comment: documents this line.
  triggeredForCurrentRed_ = false;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool ShiftAlertController::isAlertActive(TimePoint now) const {  // codex-line-comment: documents this line.
  return alertActiveUntil_ > now;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
