#include "capture/Frame.h"  // Imports project declarations from capture/Frame.h.

#include <algorithm>  // Imports the algorithm standard library declarations used in this file.
#include <cstddef>  // Imports the cstddef standard library declarations used in this file.
#include <cstdint>  // Imports the cstdint standard library declarations used in this file.

namespace fh6 {  // Places the following declarations inside namespace fh6.

FrameRegion::FrameRegion(Rect bounds, std::vector<Color> pixels)  // Begins the multi-line constructor definition for FrameRegion.
    : bounds_(bounds), pixels_(std::move(pixels)) {}  // Initializes constructor members with bounds_(bounds), pixels_(std::move(pixels)).

const Rect& FrameRegion::bounds() const {  // Implements FrameRegion::bounds.
  return bounds_;  // Returns bounds_ to the caller.
}  // Ends the current code block.

std::span<const Color> FrameRegion::pixels() const {  // Implements FrameRegion::pixels.
  return pixels_;  // Returns pixels_ to the caller.
}  // Ends the current code block.

Color FrameRegion::averageColor() const {  // Implements FrameRegion::averageColor.
  if (pixels_.empty()) {  // Guards the following work behind the condition pixels_.empty().
    return {};  // Returns {} to the caller.
  }  // Ends the current code block.

  std::uint64_t red = 0;  // Sets std::uint64_t red to 0.
  std::uint64_t green = 0;  // Sets std::uint64_t green to 0.
  std::uint64_t blue = 0;  // Sets std::uint64_t blue to 0.
  std::uint64_t alpha = 0;  // Sets std::uint64_t alpha to 0.

  for (const auto& pixel : pixels_) {  // Iterates with loop control const auto& pixel : pixels_.
    red += pixel.r;  // Sets red + to pixel.r.
    green += pixel.g;  // Sets green + to pixel.g.
    blue += pixel.b;  // Sets blue + to pixel.b.
    alpha += pixel.a;  // Sets alpha + to pixel.a.
  }  // Ends the current code block.

  const auto count = static_cast<std::uint64_t>(pixels_.size());  // Sets const auto count to static_cast<std::uint64_t>(pixels_.size()).
  return Color{  // Starts returning a Color aggregate value.
      static_cast<std::uint8_t>(red / count),  // Supplies static_cast<std::uint8_t>(red / count) to the surrounding call or initializer.
      static_cast<std::uint8_t>(green / count),  // Supplies static_cast<std::uint8_t>(green / count) to the surrounding call or initializer.
      static_cast<std::uint8_t>(blue / count),  // Supplies static_cast<std::uint8_t>(blue / count) to the surrounding call or initializer.
      static_cast<std::uint8_t>(alpha / count),  // Supplies static_cast<std::uint8_t>(alpha / count) to the surrounding call or initializer.
  };  // Ends the current type, struct, or initializer declaration.
}  // Ends the current code block.

bool FrameRegion::empty() const {  // Implements FrameRegion::empty.
  return pixels_.empty() || bounds_.empty();  // Returns pixels_.empty() || bounds_.empty() to the caller.
}  // Ends the current code block.

Frame::Frame(int width, int height, std::vector<Color> pixels, TimePoint timestamp)  // Begins the multi-line constructor definition for Frame.
    : width_(width), height_(height), pixels_(std::move(pixels)), timestamp_(timestamp) {}  // Initializes constructor members with width_(width), height_(height), pixels_(std::move(pixels)), timestamp_(timestamp).

int Frame::width() const {  // Implements Frame::width.
  return width_;  // Returns width_ to the caller.
}  // Ends the current code block.

int Frame::height() const {  // Implements Frame::height.
  return height_;  // Returns height_ to the caller.
}  // Ends the current code block.

TimePoint Frame::timestamp() const {  // Implements Frame::timestamp.
  return timestamp_;  // Returns timestamp_ to the caller.
}  // Ends the current code block.

std::span<const Color> Frame::pixels() const {  // Implements Frame::pixels.
  return pixels_;  // Returns pixels_ to the caller.
}  // Ends the current code block.

std::optional<Rect> Frame::clippedBounds(const Rect& region) const {  // Implements Frame::clippedBounds.
  if (empty() || region.empty()) {  // Guards the following work behind the condition empty() || region.empty().
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  const int left = std::clamp(region.x, 0, width_);  // Sets const int left to std::clamp(region.x, 0, width_).
  const int top = std::clamp(region.y, 0, height_);  // Sets const int top to std::clamp(region.y, 0, height_).
  const int right = std::clamp(region.x + region.width, 0, width_);  // Sets const int right to std::clamp(region.x + region.width, 0, width_).
  const int bottom = std::clamp(region.y + region.height, 0, height_);  // Sets const int bottom to std::clamp(region.y + region.height, 0, height_).

  if (right <= left || bottom <= top) {  // Guards the following work behind the condition right <= left || bottom <= top.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  return Rect{left, top, right - left, bottom - top};  // Returns Rect{left, top, right - left, bottom - top} to the caller.
}  // Ends the current code block.

std::optional<FrameRegion> Frame::crop(const Rect& region) const {  // Implements Frame::crop.
  const auto bounds = clippedBounds(region);  // Sets const auto bounds to clippedBounds(region).
  if (!bounds) {  // Guards the following work behind the condition !bounds.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  std::vector<Color> cropped(static_cast<std::size_t>(bounds->width * bounds->height));  // Declares cropped and initializes it with static_cast<std::size_t>(bounds->width * bounds->height).
  auto destination = cropped.begin();  // Sets auto destination to cropped.begin().

  for (int y = bounds->y; y < bounds->y + bounds->height; ++y) {  // Iterates with loop control int y = bounds->y; y < bounds->y + bounds->height; ++y.
    const auto rowOffset = static_cast<std::size_t>(y * width_ + bounds->x);  // Sets const auto rowOffset to static_cast<std::size_t>(y * width_ + bounds->x).
    destination = std::copy_n(pixels_.begin() + static_cast<std::ptrdiff_t>(rowOffset), bounds->width,  // Sets destination to std::copy_n(pixels_.begin() + static_cast<std::ptrdiff_t>(rowOffset), bounds->width.
                              destination);  // Supplies destination to the surrounding call or initializer.
  }  // Ends the current code block.

  return FrameRegion(*bounds, std::move(cropped));  // Returns FrameRegion(*bounds, std::move(cropped)) to the caller.
}  // Ends the current code block.

bool Frame::empty() const {  // Implements Frame::empty.
  return width_ <= 0 || height_ <= 0 ||  // Continues the surrounding declaration or control-flow expression.
         pixels_.size() != static_cast<std::size_t>(width_ * height_);  // Sets pixels_.size() ! to static_cast<std::size_t>(width_ * height_).
}  // Ends the current code block.

}  // Ends the current code block.
