#include "alert/ShiftAlertController.h"

namespace fh6 {

ShiftAlertController::ShiftAlertController(std::chrono::milliseconds arrowDuration)
    : arrowDuration_(arrowDuration) {}

ShiftAlertState ShiftAlertController::update(const GearDetectionResult& result, TimePoint now) {
  bool triggered = false;

  if (result.isConfident() && result.colorState == GearColorState::Red) {
    triggered = !triggeredForCurrentRed_;
    triggeredForCurrentRed_ = true;
    alertActiveUntil_ = now + arrowDuration_;
  } else if (result.isConfident() && result.colorState == GearColorState::White) {
    alertActiveUntil_ = now;
  }

  if (result.colorState != GearColorState::Unknown && result.isConfident()) {
    if (result.colorState == GearColorState::White) {
      triggeredForCurrentRed_ = false;
    }
    previousColorState_ = result.colorState;
  }

  return ShiftAlertState{
      isAlertActive(now),
      triggered,
      alertActiveUntil_,
  };
}

void ShiftAlertController::reset() {
  previousColorState_ = GearColorState::Unknown;
  alertActiveUntil_ = {};
  triggeredForCurrentRed_ = false;
}

bool ShiftAlertController::isAlertActive(TimePoint now) const {
  return alertActiveUntil_ > now;
}

}  // namespace fh6
