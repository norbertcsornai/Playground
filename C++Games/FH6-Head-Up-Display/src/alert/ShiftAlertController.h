#pragma once

#include <chrono>

#include "alert/ShiftAlertState.h"
#include "detection/GearDetectionResult.h"
#include "shared/GearTypes.h"
#include "shared/Time.h"

namespace fh6 {

class ShiftAlertController {
 public:
  explicit ShiftAlertController(std::chrono::milliseconds arrowDuration = std::chrono::milliseconds(1500));

  [[nodiscard]] ShiftAlertState update(const GearDetectionResult& result, TimePoint now);
  void reset();
  [[nodiscard]] bool isAlertActive(TimePoint now) const;

 private:
  GearColorState previousColorState_{GearColorState::Unknown};
  TimePoint alertActiveUntil_{};
  std::chrono::milliseconds arrowDuration_{1500};
  bool triggeredForCurrentRed_{false};
};

}  // namespace fh6
