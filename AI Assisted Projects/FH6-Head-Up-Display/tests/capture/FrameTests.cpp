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

FH6_TEST(frame_sub_rectangle_reports_source_display_size) {  // Starts a multi-line initializer or scope for frame_sub_rectangle_reports_source_display_size.
  const Frame frame(Rect{100, 50, 4, 3}, 1920, 1080, std::vector<Color>(12, Color{7, 0, 0, 255}));  // Declares a frame holding only a sub-rectangle of a 1920x1080 display.

  FH6_REQUIRE(frame.width() == 4);  // Requires the frame's own width to match the sub-rectangle.
  FH6_REQUIRE(frame.height() == 3);  // Requires the frame's own height to match the sub-rectangle.
  FH6_REQUIRE(frame.origin().x == 100);  // Requires the origin to record the sub-rectangle's display column.
  FH6_REQUIRE(frame.origin().y == 50);  // Requires the origin to record the sub-rectangle's display row.
  FH6_REQUIRE(frame.sourceWidth() == 1920);  // Requires the source width to describe the whole display.
  FH6_REQUIRE(frame.sourceHeight() == 1080);  // Requires the source height to describe the whole display.
  FH6_REQUIRE(!frame.empty());  // Requires a correctly sized sub-rectangle frame to be non-empty.
}  // Ends the current code block.

FH6_TEST(frame_sub_rectangle_crop_uses_display_coordinates) {  // Starts a multi-line initializer or scope for frame_sub_rectangle_crop_uses_display_coordinates.
  // A 4x3 sub-rectangle whose top-left sits at display coordinate (100, 50), numbered 0..11.
  std::vector<Color> pixels;  // Declares pixels for use in this scope.
  for (int index = 0; index < 4 * 3; ++index) {  // Iterates over each sub-rectangle pixel.
    pixels.push_back(Color{static_cast<std::uint8_t>(index), 0, 0, 255});  // Calls push_back on pixels.
  }  // Ends the current code block.
  const Frame frame(Rect{100, 50, 4, 3}, 1920, 1080, std::move(pixels));  // Declares a frame holding only that sub-rectangle.

  // Requesting display rect (101, 51, 2, 2) must land on buffer indices 5, 6, 9, 10.
  const auto crop = frame.crop(Rect{101, 51, 2, 2});  // Sets const auto crop to frame.crop(Rect{101, 51, 2, 2}).

  FH6_REQUIRE(crop.has_value());  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(crop->bounds().x == 101);  // Requires the crop bounds to stay in display coordinates.
  FH6_REQUIRE(crop->bounds().y == 51);  // Requires the crop bounds to stay in display coordinates.
  const auto cropped = crop->pixels();  // Sets const auto cropped to crop->pixels().
  FH6_REQUIRE(cropped.size() == 4);  // Sets FH6_REQUIRE(cropped.size() to = 4).
  FH6_REQUIRE(cropped[0].r == 5);  // Requires the crop to start at the offset-adjusted buffer index.
  FH6_REQUIRE(cropped[1].r == 6);  // Requires the crop to continue along the row.
  FH6_REQUIRE(cropped[2].r == 9);  // Requires the crop to step to the next row correctly.
  FH6_REQUIRE(cropped[3].r == 10);  // Requires the crop to finish the second row.
}  // Ends the current code block.

FH6_TEST(frame_sub_rectangle_clips_requests_outside_captured_area) {  // Starts a multi-line initializer or scope for frame_sub_rectangle_clips_requests_outside_captured_area.
  const Frame frame(Rect{100, 50, 4, 3}, 1920, 1080, std::vector<Color>(12, Color{7, 0, 0, 255}));  // Declares a frame holding only a sub-rectangle of the display.

  // Entirely outside the captured sub-rectangle, so nothing can be produced.
  FH6_REQUIRE(!frame.clippedBounds(Rect{0, 0, 10, 10}).has_value());  // Requires a fully outside request to yield no bounds.

  // Straddling the left edge clips to the captured area rather than reading out of bounds.
  const auto bounds = frame.clippedBounds(Rect{98, 50, 4, 2});  // Sets const auto bounds to frame.clippedBounds(Rect{98, 50, 4, 2}).
  FH6_REQUIRE(bounds.has_value());  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(bounds->x == 100);  // Requires the clipped left edge to snap to the captured origin.
  FH6_REQUIRE(bounds->width == 2);  // Requires the clipped width to drop the outside columns.
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
