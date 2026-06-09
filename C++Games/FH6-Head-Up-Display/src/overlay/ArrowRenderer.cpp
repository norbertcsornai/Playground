#include "overlay/ArrowRenderer.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace fh6 {

ArrowRenderer::ArrowRenderer() = default;

void ArrowRenderer::render(OverlaySurface target) const {
#ifdef _WIN32
  auto* dc = static_cast<HDC>(target);
  if (dc == nullptr) {
    return;
  }

  HPEN pen = CreatePen(PS_SOLID, 1, RGB(color_.r, color_.g, color_.b));
  HBRUSH brush = CreateSolidBrush(RGB(color_.r, color_.g, color_.b));
  HGDIOBJ oldPen = SelectObject(dc, pen);
  HGDIOBJ oldBrush = SelectObject(dc, brush);

  const int cx = size_.width / 2;
  POINT points[7] = {
      {cx, 0},
      {size_.width, size_.height / 2},
      {(size_.width * 2) / 3, size_.height / 2},
      {(size_.width * 2) / 3, size_.height},
      {size_.width / 3, size_.height},
      {size_.width / 3, size_.height / 2},
      {0, size_.height / 2},
  };
  Polygon(dc, points, 7);

  SelectObject(dc, oldBrush);
  SelectObject(dc, oldPen);
  DeleteObject(brush);
  DeleteObject(pen);
#else
  (void)target;
#endif
}

void ArrowRenderer::setColor(const Color& color) {
  color_ = color;
}

void ArrowRenderer::setSize(const Size& size) {
  if (size.width > 0 && size.height > 0) {
    size_ = size;
  }
}

Color ArrowRenderer::color() const {
  return color_;
}

Size ArrowRenderer::size() const {
  return size_;
}

}  // namespace fh6
