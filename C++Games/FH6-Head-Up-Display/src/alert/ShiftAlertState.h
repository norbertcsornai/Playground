#pragma once

#include "shared/Time.h"

namespace fh6 {

struct ShiftAlertState {
  bool active{false};
  bool triggeredThisFrame{false};
  TimePoint expiresAt{};
};

}  // namespace fh6
