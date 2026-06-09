#pragma once  // codex-line-comment: documents this line.

#include <chrono>  // codex-line-comment: documents this line.
#include <string>  // codex-line-comment: documents this line.

#include "shared/Color.h"  // codex-line-comment: documents this line.
#include "shared/Geometry.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

struct AppConfig {  // codex-line-comment: documents this line.
  Rect gearRegion{-1, -1, 420, 420};  // codex-line-comment: documents this line.
  ColorThresholds colorThresholds{};  // codex-line-comment: documents this line.
  std::chrono::milliseconds arrowDuration{1500};  // codex-line-comment: documents this line.
  Size arrowSize{75, 150};  // codex-line-comment: documents this line.
  std::string targetDisplayId{"primary"};  // codex-line-comment: documents this line.
  int captureRateLimitFps{30};  // codex-line-comment: documents this line.
  bool diagnosticsEnabled{false};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
