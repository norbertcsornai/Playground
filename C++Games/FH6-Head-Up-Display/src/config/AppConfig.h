#pragma once

#include <chrono>
#include <string>

#include "shared/Color.h"
#include "shared/Geometry.h"

namespace fh6 {

struct AppConfig {
  Rect gearRegion{-1, -1, 420, 420};
  ColorThresholds colorThresholds{};
  std::chrono::milliseconds arrowDuration{1500};
  Size arrowSize{150, 300};
  std::string targetDisplayId{"primary"};
  int captureRateLimitFps{30};
  bool diagnosticsEnabled{false};
};

}  // namespace fh6
