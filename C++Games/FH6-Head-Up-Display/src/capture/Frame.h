#pragma once  // codex-line-comment: documents this line.

#include <cstddef>  // codex-line-comment: documents this line.
#include <optional>  // codex-line-comment: documents this line.
#include <span>  // codex-line-comment: documents this line.
#include <vector>  // codex-line-comment: documents this line.

#include "shared/Color.h"  // codex-line-comment: documents this line.
#include "shared/Geometry.h"  // codex-line-comment: documents this line.
#include "shared/Time.h"  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

class FrameRegion {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  FrameRegion(Rect bounds, std::vector<Color> pixels);  // codex-line-comment: documents this line.

  [[nodiscard]] const Rect& bounds() const;  // codex-line-comment: documents this line.
  [[nodiscard]] std::span<const Color> pixels() const;  // codex-line-comment: documents this line.
  [[nodiscard]] Color averageColor() const;  // codex-line-comment: documents this line.
  [[nodiscard]] bool empty() const;  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  Rect bounds_{};  // codex-line-comment: documents this line.
  std::vector<Color> pixels_{};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

class Frame {  // codex-line-comment: documents this line.
 public:  // codex-line-comment: documents this line.
  Frame() = default;  // codex-line-comment: documents this line.
  Frame(int width, int height, std::vector<Color> pixels, TimePoint timestamp = Clock::now());  // codex-line-comment: documents this line.

  [[nodiscard]] int width() const;  // codex-line-comment: documents this line.
  [[nodiscard]] int height() const;  // codex-line-comment: documents this line.
  [[nodiscard]] TimePoint timestamp() const;  // codex-line-comment: documents this line.
  [[nodiscard]] std::span<const Color> pixels() const;  // codex-line-comment: documents this line.
  [[nodiscard]] std::optional<FrameRegion> crop(const Rect& region) const;  // codex-line-comment: documents this line.
  [[nodiscard]] bool empty() const;  // codex-line-comment: documents this line.

 private:  // codex-line-comment: documents this line.
  int width_{0};  // codex-line-comment: documents this line.
  int height_{0};  // codex-line-comment: documents this line.
  std::vector<Color> pixels_{};  // codex-line-comment: documents this line.
  TimePoint timestamp_{Clock::now()};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
