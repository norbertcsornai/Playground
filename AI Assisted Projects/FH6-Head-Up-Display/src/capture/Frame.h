#pragma once  // Prevents this header from being included more than once.

#include <cstddef>  // Imports the cstddef standard library declarations used in this file.
#include <optional>  // Imports the optional standard library declarations used in this file.
#include <span>  // Imports the span standard library declarations used in this file.
#include <vector>  // Imports the vector standard library declarations used in this file.

#include "shared/Color.h"  // Imports project declarations from shared/Color.h.
#include "shared/Geometry.h"  // Imports project declarations from shared/Geometry.h.
#include "shared/Time.h"  // Imports project declarations from shared/Time.h.

namespace fh6 {  // Places the following declarations inside namespace fh6.

class FrameRegion {  // Declares the FrameRegion class interface and members.
 public:  // Makes the following members part of the public API.
  FrameRegion(Rect bounds, std::vector<Color> pixels);  // Invokes FrameRegion with the supplied arguments.

  [[nodiscard]] const Rect& bounds() const;  // Declares bounds and marks its return value as important.
  [[nodiscard]] std::span<const Color> pixels() const;  // Declares pixels and marks its return value as important.
  [[nodiscard]] Color averageColor() const;  // Declares averageColor and marks its return value as important.
  [[nodiscard]] bool empty() const;  // Declares empty and marks its return value as important.

 private:  // Makes the following members private implementation details.
  Rect bounds_{};  // Declares bounds_ with value initialization.
  std::vector<Color> pixels_{};  // Declares pixels_ with value initialization.
};  // Ends the current type, struct, or initializer declaration.

// A Frame usually holds a whole captured display, but it may instead hold only a sub-rectangle of
// one (see DesktopFrameCapture's region-of-interest support). Rects passed to clippedBounds() and
// crop() are always in display coordinates regardless of which case applies: origin() says where
// this frame's pixels begin in that space, and sourceWidth()/sourceHeight() report the size of the
// display the pixels came from, so callers that derive regions from display size stay correct.
class Frame {  // Declares the Frame class interface and members.
 public:  // Makes the following members part of the public API.
  Frame() = default;  // Sets Frame() to default.
  Frame(int width, int height, std::vector<Color> pixels, TimePoint timestamp = Clock::now());  // Sets Frame(int width, int height, std::vector<Color> pixels, TimePoint timestamp to Clock::now()).
  Frame(Rect origin, int sourceWidth, int sourceHeight, std::vector<Color> pixels,  // Supplies the sub-rectangle constructor parameters to the surrounding declaration.
        TimePoint timestamp = Clock::now());  // Sets TimePoint timestamp to Clock::now().

  [[nodiscard]] int width() const;  // Declares width and marks its return value as important.
  [[nodiscard]] int height() const;  // Declares height and marks its return value as important.
  [[nodiscard]] const Rect& origin() const;  // Declares origin and marks its return value as important.
  [[nodiscard]] int sourceWidth() const;  // Declares sourceWidth and marks its return value as important.
  [[nodiscard]] int sourceHeight() const;  // Declares sourceHeight and marks its return value as important.
  [[nodiscard]] TimePoint timestamp() const;  // Declares timestamp and marks its return value as important.
  [[nodiscard]] std::span<const Color> pixels() const;  // Declares pixels and marks its return value as important.
  [[nodiscard]] std::optional<Rect> clippedBounds(const Rect& region) const;  // Declares clippedBounds and marks its return value as important.
  [[nodiscard]] std::optional<FrameRegion> crop(const Rect& region) const;  // Declares crop and marks its return value as important.
  [[nodiscard]] bool empty() const;  // Declares empty and marks its return value as important.

 private:  // Makes the following members private implementation details.
  int width_{0};  // Declares width_ and initializes it with 0.
  int height_{0};  // Declares height_ and initializes it with 0.
  Rect origin_{};  // Declares where this frame's pixels start in display coordinates.
  int sourceWidth_{0};  // Declares the width of the display these pixels were captured from.
  int sourceHeight_{0};  // Declares the height of the display these pixels were captured from.
  std::vector<Color> pixels_{};  // Declares pixels_ with value initialization.
  TimePoint timestamp_{Clock::now()};  // Declares timestamp_ and initializes it with Clock::now().
};  // Ends the current type, struct, or initializer declaration.

}  // Ends the current code block.
