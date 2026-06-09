#pragma once

#include <string>

#include "shared/Geometry.h"

namespace fh6 {

using WindowHandle = void*;

struct DisplayInfo {
  std::string id{"primary"};
  Rect bounds{};
  float dpiScale{1.0F};
  bool isHdrEnabled{false};
};

struct WindowInfo {
  WindowHandle handle{nullptr};
  std::string title{};
  Rect bounds{};
  DisplayInfo display{};
};

}  // namespace fh6
