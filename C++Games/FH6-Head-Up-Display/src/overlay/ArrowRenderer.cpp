#include "overlay/ArrowRenderer.h"  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
#include <windows.h>  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

ArrowRenderer::ArrowRenderer() = default;  // codex-line-comment: documents this line.

void ArrowRenderer::render(OverlaySurface target) const {  // codex-line-comment: documents this line.
#ifdef _WIN32  // codex-line-comment: documents this line.
  auto* dc = static_cast<HDC>(target);  // codex-line-comment: documents this line.
  if (dc == nullptr) {  // codex-line-comment: documents this line.
    return;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  HPEN pen = CreatePen(PS_SOLID, 1, RGB(color_.r, color_.g, color_.b));  // codex-line-comment: documents this line.
  HBRUSH brush = CreateSolidBrush(RGB(color_.r, color_.g, color_.b));  // codex-line-comment: documents this line.
  HGDIOBJ oldPen = SelectObject(dc, pen);  // codex-line-comment: documents this line.
  HGDIOBJ oldBrush = SelectObject(dc, brush);  // codex-line-comment: documents this line.

  const int cx = size_.width / 2;  // codex-line-comment: documents this line.
  POINT points[7] = {  // codex-line-comment: documents this line.
      {cx, 0},  // codex-line-comment: documents this line.
      {size_.width, size_.height / 2},  // codex-line-comment: documents this line.
      {(size_.width * 2) / 3, size_.height / 2},  // codex-line-comment: documents this line.
      {(size_.width * 2) / 3, size_.height},  // codex-line-comment: documents this line.
      {size_.width / 3, size_.height},  // codex-line-comment: documents this line.
      {size_.width / 3, size_.height / 2},  // codex-line-comment: documents this line.
      {0, size_.height / 2},  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.
  Polygon(dc, points, 7);  // codex-line-comment: documents this line.

  SelectObject(dc, oldBrush);  // codex-line-comment: documents this line.
  SelectObject(dc, oldPen);  // codex-line-comment: documents this line.
  DeleteObject(brush);  // codex-line-comment: documents this line.
  DeleteObject(pen);  // codex-line-comment: documents this line.
#else  // codex-line-comment: documents this line.
  (void)target;  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void ArrowRenderer::setColor(const Color& color) {  // codex-line-comment: documents this line.
  color_ = color;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void ArrowRenderer::setSize(const Size& size) {  // codex-line-comment: documents this line.
  if (size.width > 0 && size.height > 0) {  // codex-line-comment: documents this line.
    size_ = size;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

Color ArrowRenderer::color() const {  // codex-line-comment: documents this line.
  return color_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

Size ArrowRenderer::size() const {  // codex-line-comment: documents this line.
  return size_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
