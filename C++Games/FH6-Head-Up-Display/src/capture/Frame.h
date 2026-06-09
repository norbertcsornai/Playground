#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

#include "shared/Color.h"
#include "shared/Geometry.h"
#include "shared/Time.h"

namespace fh6 {

class FrameRegion {
 public:
  FrameRegion(Rect bounds, std::vector<Color> pixels);

  [[nodiscard]] const Rect& bounds() const;
  [[nodiscard]] std::span<const Color> pixels() const;
  [[nodiscard]] Color averageColor() const;
  [[nodiscard]] bool empty() const;

 private:
  Rect bounds_{};
  std::vector<Color> pixels_{};
};

class Frame {
 public:
  Frame() = default;
  Frame(int width, int height, std::vector<Color> pixels, TimePoint timestamp = Clock::now());

  [[nodiscard]] int width() const;
  [[nodiscard]] int height() const;
  [[nodiscard]] TimePoint timestamp() const;
  [[nodiscard]] std::span<const Color> pixels() const;
  [[nodiscard]] std::optional<FrameRegion> crop(const Rect& region) const;
  [[nodiscard]] bool empty() const;

 private:
  int width_{0};
  int height_{0};
  std::vector<Color> pixels_{};
  TimePoint timestamp_{Clock::now()};
};

}  // namespace fh6
