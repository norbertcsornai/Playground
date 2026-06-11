#pragma once  // Prevents this header from being included more than once.

namespace fh6 {  // Places the following declarations inside namespace fh6.

struct Rect {  // Declares the Rect value type and fields.
  int x{0};  // Declares x and initializes it with 0.
  int y{0};  // Declares y and initializes it with 0.
  int width{0};  // Declares width and initializes it with 0.
  int height{0};  // Declares height and initializes it with 0.

  [[nodiscard]] bool empty() const {  // Begins function empty.
    return width <= 0 || height <= 0;  // Returns width <= 0 || height <= 0 to the caller.
  }  // Ends the current code block.
};  // Ends the current type, struct, or initializer declaration.

struct Size {  // Declares the Size value type and fields.
  int width{0};  // Declares width and initializes it with 0.
  int height{0};  // Declares height and initializes it with 0.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
