#pragma once  // Prevents this header from being included more than once.

#include "shared/Color.h"  // Imports project declarations from shared/Color.h.
#include "shared/Geometry.h"  // Imports project declarations from shared/Geometry.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

using OverlaySurface = void*;  // Aliases OverlaySurface to void*.

class ArrowRenderer {  // Declares the ArrowRenderer class interface and members.
 public:  // Makes the following members part of the public API.
  ArrowRenderer();  // Invokes ArrowRenderer with the supplied arguments.

  void render(OverlaySurface target) const;  // Declares function render for callers.
  void setColor(const Color& color);  // Declares function setColor for callers.
  void setSize(const Size& size);  // Declares function setSize for callers.

  [[nodiscard]] Color color() const;  // Declares color and marks its return value as important.
  [[nodiscard]] Size size() const;  // Declares size and marks its return value as important.

 private:  // Makes the following members private implementation details.
  Color color_{255, 0, 0, 255};  // Declares color_ and initializes it with 255, 0, 0, 255.
  Size size_{50, 100};  // Declares size_ and initializes it with 50, 100.
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
