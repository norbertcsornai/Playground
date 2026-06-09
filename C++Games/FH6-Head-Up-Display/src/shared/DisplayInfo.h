#pragma once  // codex-line-comment: documents this line.

#include <string>  // codex-line-comment: documents this line.

#include "shared/Geometry.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

using WindowHandle = void*;  // codex-line-comment: documents this line.

struct DisplayInfo {  // codex-line-comment: documents this line.
  std::string id{"primary"};  // codex-line-comment: documents this line.
  Rect bounds{};  // codex-line-comment: documents this line.
  float dpiScale{1.0F};  // codex-line-comment: documents this line.
  bool isHdrEnabled{false};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

struct WindowInfo {  // codex-line-comment: documents this line.
  WindowHandle handle{nullptr};  // codex-line-comment: documents this line.
  std::string title{};  // codex-line-comment: documents this line.
  Rect bounds{};  // codex-line-comment: documents this line.
  DisplayInfo display{};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
