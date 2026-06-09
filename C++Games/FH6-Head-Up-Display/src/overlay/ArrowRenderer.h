#pragma once  // codex-line-comment: documents this line.

#include "shared/Color.h"  // codex-line-comment: documents this line.
#include "shared/Geometry.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

using OverlaySurface = void*;  // codex-line-comment: documents this line.

class ArrowRenderer {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  ArrowRenderer();  // codex-line-comment: documents this line.

  void render(OverlaySurface target) const;  // codex-line-comment: documents this line.
  void setColor(const Color& color);  // codex-line-comment: documents this line.
  void setSize(const Size& size);  // codex-line-comment: documents this line.

  [[nodiscard]] Color color() const;  // codex-line-comment: documents this line.
  [[nodiscard]] Size size() const;  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  Color color_{255, 0, 0, 255};  // codex-line-comment: documents this line.
  Size size_{50, 100};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
