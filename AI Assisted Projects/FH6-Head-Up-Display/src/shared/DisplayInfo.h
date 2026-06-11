#pragma once  // Prevents this header from being included more than once.

#include <string>  // Imports the string standard library declarations used in this file.

#include "shared/Geometry.h"  // Imports project declarations from shared/Geometry.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

using WindowHandle = void*;  // Aliases WindowHandle to void*.

struct DisplayInfo {  // Declares the DisplayInfo value type and fields.
  std::string id{"primary"};  // Declares id and initializes it with "primary".
  Rect bounds{};  // Declares bounds with value initialization.
  float dpiScale{1.0F};  // Declares dpiScale and initializes it with 1.0F.
  bool isHdrEnabled{false};  // Declares isHdrEnabled and initializes it with false.
};  // Ends the current type, struct, or initializer declaration.

struct WindowInfo {  // Declares the WindowInfo value type and fields.
  WindowHandle handle{nullptr};  // Declares handle and initializes it with nullptr.
  std::string title{};  // Declares title with value initialization.
  Rect bounds{};  // Declares bounds with value initialization.
  DisplayInfo display{};  // Declares display with value initialization.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
