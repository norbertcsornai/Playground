#pragma once  // codex-line-comment: documents this line.

#include "shared/Time.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

struct ShiftAlertState {  // codex-line-comment: documents this line.
  bool active{false};  // codex-line-comment: documents this line.
  bool triggeredThisFrame{false};  // codex-line-comment: documents this line.
  TimePoint expiresAt{};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
