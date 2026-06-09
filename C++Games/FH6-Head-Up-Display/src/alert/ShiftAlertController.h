#pragma once  // codex-line-comment: documents this line.

#include <chrono>  // codex-line-comment: documents this line.

#include "alert/ShiftAlertState.h"  // codex-line-comment: documents this line.
#include "detection/GearDetectionResult.h"  // codex-line-comment: documents this line.
#include "shared/GearTypes.h"  // codex-line-comment: documents this line.
#include "shared/Time.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class ShiftAlertController {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  explicit ShiftAlertController(std::chrono::milliseconds arrowDuration = std::chrono::milliseconds(1500));  // codex-line-comment: documents this line.

  [[nodiscard]] ShiftAlertState update(const GearDetectionResult& result, TimePoint now);  // codex-line-comment: documents this line.
  void reset();  // codex-line-comment: documents this line.
  [[nodiscard]] bool isAlertActive(TimePoint now) const;  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  GearColorState previousColorState_{GearColorState::Unknown};  // codex-line-comment: documents this line.
  TimePoint alertActiveUntil_{};  // codex-line-comment: documents this line.
  std::chrono::milliseconds arrowDuration_{1500};  // codex-line-comment: documents this line.
  bool triggeredForCurrentRed_{false};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
