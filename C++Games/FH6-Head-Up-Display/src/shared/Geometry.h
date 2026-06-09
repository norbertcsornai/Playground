#pragma once  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

struct Rect {  // codex-line-comment: documents this line.
  int x{0};  // codex-line-comment: documents this line.
  int y{0};  // codex-line-comment: documents this line.
  int width{0};  // codex-line-comment: documents this line.
  int height{0};  // codex-line-comment: documents this line.

  [[nodiscard]] bool empty() const {  // codex-line-comment: documents this line.
    return width <= 0 || height <= 0;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

struct Size {  // codex-line-comment: documents this line.
  int width{0};  // codex-line-comment: documents this line.
  int height{0};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
