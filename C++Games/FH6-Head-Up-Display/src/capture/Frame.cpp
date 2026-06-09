#include "capture/Frame.h"

#include <algorithm>
#include <cstdint>

namespace fh6 {

FrameRegion::FrameRegion(Rect bounds, std::vector<Color> pixels)
    : bounds_(bounds), pixels_(std::move(pixels)) {}

const Rect& FrameRegion::bounds() const {
  return bounds_;
}

std::span<const Color> FrameRegion::pixels() const {
  return pixels_;
}

Color FrameRegion::averageColor() const {
  if (pixels_.empty()) {
    return {};
  }

  std::uint64_t red = 0;
  std::uint64_t green = 0;
  std::uint64_t blue = 0;
  std::uint64_t alpha = 0;

  for (const auto& pixel : pixels_) {
    red += pixel.r;
    green += pixel.g;
    blue += pixel.b;
    alpha += pixel.a;
  }

  const auto count = static_cast<std::uint64_t>(pixels_.size());
  return Color{
      static_cast<std::uint8_t>(red / count),
      static_cast<std::uint8_t>(green / count),
      static_cast<std::uint8_t>(blue / count),
      static_cast<std::uint8_t>(alpha / count),
  };
}

bool FrameRegion::empty() const {
  return pixels_.empty() || bounds_.empty();
}

Frame::Frame(int width, int height, std::vector<Color> pixels, TimePoint timestamp)
    : width_(width), height_(height), pixels_(std::move(pixels)), timestamp_(timestamp) {}

int Frame::width() const {
  return width_;
}

int Frame::height() const {
  return height_;
}

TimePoint Frame::timestamp() const {
  return timestamp_;
}

std::span<const Color> Frame::pixels() const {
  return pixels_;
}

std::optional<FrameRegion> Frame::crop(const Rect& region) const {
  if (empty() || region.empty()) {
    return std::nullopt;
  }

  const int left = std::clamp(region.x, 0, width_);
  const int top = std::clamp(region.y, 0, height_);
  const int right = std::clamp(region.x + region.width, 0, width_);
  const int bottom = std::clamp(region.y + region.height, 0, height_);

  if (right <= left || bottom <= top) {
    return std::nullopt;
  }

  std::vector<Color> cropped;
  cropped.reserve(static_cast<std::size_t>((right - left) * (bottom - top)));

  for (int y = top; y < bottom; ++y) {
    const auto rowOffset = static_cast<std::size_t>(y * width_);
    for (int x = left; x < right; ++x) {
      cropped.push_back(pixels_[rowOffset + static_cast<std::size_t>(x)]);
    }
  }

  return FrameRegion(Rect{left, top, right - left, bottom - top}, std::move(cropped));
}

bool Frame::empty() const {
  return width_ <= 0 || height_ <= 0 ||
         pixels_.size() != static_cast<std::size_t>(width_ * height_);
}

}  // namespace fh6
