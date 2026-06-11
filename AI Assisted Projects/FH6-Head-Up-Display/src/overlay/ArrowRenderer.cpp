#include "overlay/ArrowRenderer.h"  // Imports project declarations from overlay/ArrowRenderer.h.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
#include <windows.h>  // Imports the windows.h standard library declarations used in this file.
#endif  // Ends the compile-time selection block.

namespace fh6 {  // Places the following declarations inside namespace fh6.

ArrowRenderer::ArrowRenderer() = default;  // Sets ArrowRenderer::ArrowRenderer() to default.

void ArrowRenderer::render(OverlaySurface target) const {  // Implements ArrowRenderer::render.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  auto* dc = static_cast<HDC>(target);  // Sets auto* dc to static_cast<HDC>(target).
  if (dc == nullptr) {  // Guards the following work behind the condition dc == nullptr.
    return;  // Leaves this function without a return value.
  }  // Ends the current code block.

  HPEN pen = CreatePen(PS_SOLID, 1, RGB(color_.r, color_.g, color_.b));  // Sets HPEN pen to CreatePen(PS_SOLID, 1, RGB(color_.r, color_.g, color_.b)).
  HBRUSH brush = CreateSolidBrush(RGB(color_.r, color_.g, color_.b));  // Sets HBRUSH brush to CreateSolidBrush(RGB(color_.r, color_.g, color_.b)).
  HGDIOBJ oldPen = SelectObject(dc, pen);  // Sets HGDIOBJ oldPen to SelectObject(dc, pen).
  HGDIOBJ oldBrush = SelectObject(dc, brush);  // Sets HGDIOBJ oldBrush to SelectObject(dc, brush).

  const int cx = size_.width / 2;  // Sets const int cx to size_.width / 2.
  POINT points[7] = {  // Starts a multi-line initializer or scope for POINT points[7] =.
      {cx, 0},  // Finishes this initializer entry for the surrounding aggregate.
      {size_.width, size_.height / 2},  // Finishes this initializer entry for the surrounding aggregate.
      {(size_.width * 2) / 3, size_.height / 2},  // Finishes this initializer entry for the surrounding aggregate.
      {(size_.width * 2) / 3, size_.height},  // Finishes this initializer entry for the surrounding aggregate.
      {size_.width / 3, size_.height},  // Finishes this initializer entry for the surrounding aggregate.
      {size_.width / 3, size_.height / 2},  // Finishes this initializer entry for the surrounding aggregate.
      {0, size_.height / 2},  // Finishes this initializer entry for the surrounding aggregate.
  };  // Ends the current type, struct, or initializer declaration.
  Polygon(dc, points, 7);  // Invokes Polygon with the supplied arguments.

  SelectObject(dc, oldBrush);  // Invokes SelectObject with the supplied arguments.
  SelectObject(dc, oldPen);  // Invokes SelectObject with the supplied arguments.
  DeleteObject(brush);  // Invokes DeleteObject with the supplied arguments.
  DeleteObject(pen);  // Invokes DeleteObject with the supplied arguments.
#else  // Selects this compile-time branch when earlier branches were not selected.
  (void)target;  // Marks target as intentionally unused in this build path.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

void ArrowRenderer::setColor(const Color& color) {  // Implements ArrowRenderer::setColor.
  color_ = color;  // Sets color_ to color.
}  // Ends the current code block.

void ArrowRenderer::setSize(const Size& size) {  // Implements ArrowRenderer::setSize.
  if (size.width > 0 && size.height > 0) {  // Guards the following work behind the condition size.width > 0 && size.height > 0.
    size_ = size;  // Sets size_ to size.
  }  // Ends the current code block.
}  // Ends the current code block.

Color ArrowRenderer::color() const {  // Implements ArrowRenderer::color.
  return color_;  // Returns color_ to the caller.
}  // Ends the current code block.

Size ArrowRenderer::size() const {  // Implements ArrowRenderer::size.
  return size_;  // Returns size_ to the caller.
}  // Ends the current code block.

}  // Ends the current code block.
