#pragma once  // Prevents this header from being included more than once.

namespace fh6 {  // Places the following declarations inside namespace fh6.

enum class FrameStatus {  // Declares the FrameStatus scoped enum values.
  Idle,  // Supplies Idle to the surrounding call or initializer.
  Capturing,  // Supplies Capturing to the surrounding call or initializer.
  Unavailable,  // Supplies Unavailable to the surrounding call or initializer.
  Failed,  // Supplies Failed to the surrounding call or initializer.
};  // Ends the current type, struct, or initializer declaration.

enum class OverlayStatus {  // Declares the OverlayStatus scoped enum values.
  Hidden,  // Supplies Hidden to the surrounding call or initializer.
  Visible,  // Supplies Visible to the surrounding call or initializer.
  Failed,  // Supplies Failed to the surrounding call or initializer.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
