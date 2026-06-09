#include "capture/Frame.h"  // codex-line-comment: documents this line.

#include <algorithm>  // codex-line-comment: documents this line.
#include <cstdint>  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

FrameRegion::FrameRegion(Rect bounds, std::vector<Color> pixels)  // codex-line-comment: documents this line.
    : bounds_(bounds), pixels_(std::move(pixels)) {}  // codex-line-comment: documents this line.

const Rect& FrameRegion::bounds() const {  // codex-line-comment: documents this line.
  return bounds_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

std::span<const Color> FrameRegion::pixels() const {  // codex-line-comment: documents this line.
  return pixels_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

Color FrameRegion::averageColor() const {  // codex-line-comment: documents this line.
  if (pixels_.empty()) {  // codex-line-comment: documents this line.
    return {};  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  std::uint64_t red = 0;  // codex-line-comment: documents this line.
  std::uint64_t green = 0;  // codex-line-comment: documents this line.
  std::uint64_t blue = 0;  // codex-line-comment: documents this line.
  std::uint64_t alpha = 0;  // codex-line-comment: documents this line.

  for (const auto& pixel : pixels_) {  // codex-line-comment: documents this line.
    red += pixel.r;  // codex-line-comment: documents this line.
    green += pixel.g;  // codex-line-comment: documents this line.
    blue += pixel.b;  // codex-line-comment: documents this line.
    alpha += pixel.a;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  const auto count = static_cast<std::uint64_t>(pixels_.size());  // codex-line-comment: documents this line.
  return Color{  // codex-line-comment: documents this line.
      static_cast<std::uint8_t>(red / count),  // codex-line-comment: documents this line.
      static_cast<std::uint8_t>(green / count),  // codex-line-comment: documents this line.
      static_cast<std::uint8_t>(blue / count),  // codex-line-comment: documents this line.
      static_cast<std::uint8_t>(alpha / count),  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool FrameRegion::empty() const {  // codex-line-comment: documents this line.
  return pixels_.empty() || bounds_.empty();  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

Frame::Frame(int width, int height, std::vector<Color> pixels, TimePoint timestamp)  // codex-line-comment: documents this line.
    : width_(width), height_(height), pixels_(std::move(pixels)), timestamp_(timestamp) {}  // codex-line-comment: documents this line.

int Frame::width() const {  // codex-line-comment: documents this line.
  return width_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

int Frame::height() const {  // codex-line-comment: documents this line.
  return height_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

TimePoint Frame::timestamp() const {  // codex-line-comment: documents this line.
  return timestamp_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

std::span<const Color> Frame::pixels() const {  // codex-line-comment: documents this line.
  return pixels_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

std::optional<FrameRegion> Frame::crop(const Rect& region) const {  // codex-line-comment: documents this line.
  if (empty() || region.empty()) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  const int left = std::clamp(region.x, 0, width_);  // codex-line-comment: documents this line.
  const int top = std::clamp(region.y, 0, height_);  // codex-line-comment: documents this line.
  const int right = std::clamp(region.x + region.width, 0, width_);  // codex-line-comment: documents this line.
  const int bottom = std::clamp(region.y + region.height, 0, height_);  // codex-line-comment: documents this line.

  if (right <= left || bottom <= top) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  std::vector<Color> cropped;  // codex-line-comment: documents this line.
  cropped.reserve(static_cast<std::size_t>((right - left) * (bottom - top)));  // codex-line-comment: documents this line.

  for (int y = top; y < bottom; ++y) {  // codex-line-comment: documents this line.
    const auto rowOffset = static_cast<std::size_t>(y * width_);  // codex-line-comment: documents this line.
    for (int x = left; x < right; ++x) {  // codex-line-comment: documents this line.
      cropped.push_back(pixels_[rowOffset + static_cast<std::size_t>(x)]);  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return FrameRegion(Rect{left, top, right - left, bottom - top}, std::move(cropped));  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool Frame::empty() const {  // codex-line-comment: documents this line.
  return width_ <= 0 || height_ <= 0 ||  // codex-line-comment: documents this line.
         pixels_.size() != static_cast<std::size_t>(width_ * height_);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
