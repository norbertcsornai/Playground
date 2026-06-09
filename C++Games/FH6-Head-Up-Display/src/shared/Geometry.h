#pragma once

namespace fh6 {

struct Rect {
  int x{0};
  int y{0};
  int width{0};
  int height{0};

  [[nodiscard]] bool empty() const {
    return width <= 0 || height <= 0;
  }
};

struct Size {
  int width{0};
  int height{0};
};

}  // namespace fh6
