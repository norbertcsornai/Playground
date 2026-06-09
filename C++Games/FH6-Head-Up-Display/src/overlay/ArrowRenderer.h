#pragma once

#include "shared/Color.h"
#include "shared/Geometry.h"

namespace fh6 {

using OverlaySurface = void*;

class ArrowRenderer {
 public:
  ArrowRenderer();

  void render(OverlaySurface target) const;
  void setColor(const Color& color);
  void setSize(const Size& size);

  [[nodiscard]] Color color() const;
  [[nodiscard]] Size size() const;

 private:
  Color color_{255, 0, 0, 255};
  Size size_{50, 100};
};

}  // namespace fh6
