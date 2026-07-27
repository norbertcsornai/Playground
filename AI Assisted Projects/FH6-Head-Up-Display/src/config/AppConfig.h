#pragma once  // Prevents this header from being included more than once.

#include <chrono>  // Imports the chrono standard library declarations used in this file.
#include <string>  // Imports the string standard library declarations used in this file.

#include "shared/Color.h"  // Imports project declarations from shared/Color.h.
#include "shared/Geometry.h"  // Imports project declarations from shared/Geometry.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

struct AppConfig {  // Declares the AppConfig value type and fields.
  Rect gearRegion{-1, -1, 420, 420};  // Declares gearRegion and initializes it with -1, -1, 420, 420.
  ColorThresholds colorThresholds{};  // Declares colorThresholds with value initialization.
  std::chrono::milliseconds arrowDuration{1500};  // Declares arrowDuration and initializes it with 1500.
  Size arrowSize{75, 150};  // Declares arrowSize and initializes it with 75, 150.
  std::string targetDisplayId{"primary"};  // Declares targetDisplayId and initializes it with "primary".
  int captureRateLimitFps{120};  // Declares captureRateLimitFps and initializes it with 120.
  bool diagnosticsEnabled{false};  // Declares diagnosticsEnabled and initializes it with false.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
