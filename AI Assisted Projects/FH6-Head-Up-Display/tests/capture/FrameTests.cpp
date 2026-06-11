#include "TestFramework.h"  // Imports project declarations from TestFramework.h.

#include <cstdint>  // Imports the cstdint standard library declarations used in this file.
#include <vector>  // Imports the vector standard library declarations used in this file.

#include "capture/Frame.h"  // Imports project declarations from capture/Frame.h.

using namespace fh6;  // Places fh6 symbols into this test translation unit.

namespace {  // Starts a file-local helper namespace.

Frame numberedFrame(int width, int height) {  // Begins function numberedFrame.
  std::vector<Color> pixels;  // Declares pixels for use in this scope.
  pixels.reserve(static_cast<std::size_t>(width * height));  // Calls reserve on pixels.
  for (int index = 0; index < width * height; ++index) {  // Iterates with loop control int index = 0; index < width * height; ++index.
    pixels.push_back(Color{static_cast<std::uint8_t>(index), 0, 0, 255});  // Calls push_back on pixels.
  }  // Ends the current code block.
  return Frame(width, height, std::move(pixels));  // Returns Frame(width, height, std::move(pixels)) to the caller.
}  // Ends the current code block.

}  // Ends the current code block.

FH6_TEST(frame_clipped_bounds_clamps_to_frame_edges) {  // Starts a multi-line initializer or scope for FH6_TEST(frame_clipped_bounds_clamps_to_frame_edges).
  const auto frame = numberedFrame(10, 10);  // Sets const auto frame to numberedFrame(10, 10).

  const auto bounds = frame.clippedBounds(Rect{-2, 3, 5, 4});  // Sets const auto bounds to frame.clippedBounds(Rect{-2, 3, 5, 4}).

  FH6_REQUIRE(bounds.has_value());  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(bounds->x == 0);  // Sets FH6_REQUIRE(bounds->x to = 0).
  FH6_REQUIRE(bounds->y == 3);  // Sets FH6_REQUIRE(bounds->y to = 3).
  FH6_REQUIRE(bounds->width == 3);  // Sets FH6_REQUIRE(bounds->width to = 3).
  FH6_REQUIRE(bounds->height == 4);  // Sets FH6_REQUIRE(bounds->height to = 4).
}  // Ends the current code block.

FH6_TEST(frame_crop_copies_rows_in_display_order) {  // Starts a multi-line initializer or scope for FH6_TEST(frame_crop_copies_rows_in_display_order).
  const auto frame = numberedFrame(4, 3);  // Sets const auto frame to numberedFrame(4, 3).

  const auto crop = frame.crop(Rect{1, 1, 2, 2});  // Sets const auto crop to frame.crop(Rect{1, 1, 2, 2}).

  FH6_REQUIRE(crop.has_value());  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(crop->bounds().x == 1);  // Sets FH6_REQUIRE(crop->bounds().x to = 1).
  FH6_REQUIRE(crop->bounds().y == 1);  // Sets FH6_REQUIRE(crop->bounds().y to = 1).
  FH6_REQUIRE(crop->bounds().width == 2);  // Sets FH6_REQUIRE(crop->bounds().width to = 2).
  FH6_REQUIRE(crop->bounds().height == 2);  // Sets FH6_REQUIRE(crop->bounds().height to = 2).

  const auto pixels = crop->pixels();  // Sets const auto pixels to crop->pixels().
  FH6_REQUIRE(pixels.size() == 4);  // Sets FH6_REQUIRE(pixels.size() to = 4).
  FH6_REQUIRE(pixels[0].r == 5);  // Sets FH6_REQUIRE(pixels[0].r to = 5).
  FH6_REQUIRE(pixels[1].r == 6);  // Sets FH6_REQUIRE(pixels[1].r to = 6).
  FH6_REQUIRE(pixels[2].r == 9);  // Sets FH6_REQUIRE(pixels[2].r to = 9).
  FH6_REQUIRE(pixels[3].r == 10);  // Sets FH6_REQUIRE(pixels[3].r to = 10).
}  // Ends the current code block.
