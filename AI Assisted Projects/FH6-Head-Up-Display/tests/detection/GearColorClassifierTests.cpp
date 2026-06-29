#include "TestFramework.h"  // Imports project declarations from TestFramework.h.

#include <vector>  // Imports the vector standard library declarations used in this file.

#include "capture/Frame.h"  // Imports project declarations from capture/Frame.h.
#include "detection/GearColorClassifier.h"  // Imports project declarations from detection/GearColorClassifier.h.

using namespace fh6;  // Declares fh6 for use in this scope.

namespace {  // Starts a file-local helper namespace.

FrameRegion regionWithColor(Color color) {  // Begins function regionWithColor.
  constexpr int size = 80;  // Defines compile-time constant size as 80.
  return FrameRegion(Rect{0, 0, size, size},  // Finishes this initializer entry for the surrounding aggregate.
                     std::vector<Color>(static_cast<std::size_t>(size * size), color));  // Executes std::vector<Color>(static_cast<std::size_t>(size * size), color)).
}  // Ends the current code block.

FrameRegion sparseRegionWithGlyph(Color glyphColor) {  // Begins function sparseRegionWithGlyph.
  std::vector<Color> pixels(260 * 220, Color{10, 10, 10, 255});  // Declares function pixels for callers.
  for (int y = 90; y < 98; ++y) {  // Iterates with loop control int y = 90; y < 98; ++y.
    for (int x = 120; x < 130; ++x) {  // Iterates with loop control int x = 120; x < 130; ++x.
      pixels[static_cast<std::size_t>(y * 260 + x)] = glyphColor;  // Sets pixels[static_cast<std::size_t>(y * 260 + x)] to glyphColor.
    }  // Ends the current code block.
  }  // Ends the current code block.
  return FrameRegion(Rect{0, 0, 260, 220}, std::move(pixels));  // Returns FrameRegion(Rect{0, 0, 260, 220}, std::move(pixels)) to the caller.
}  // Ends the current code block.

FrameRegion widgetRegion(Color widgetColor, Color backgroundColor = Color{10, 10, 10, 255}) {  // Begins function widgetRegion.
  constexpr int size = 160;  // Defines compile-time constant size as 160.
  constexpr int center = size / 2;  // Defines compile-time constant center as size / 2.
  constexpr int ringInnerSquared = 36 * 36;  // Defines the squared inner radius of the synthetic gear ring.
  constexpr int ringOuterSquared = 50 * 50;  // Defines the squared outer radius of the synthetic gear ring.
  constexpr int digitHalfWidth = 4;  // Defines half the synthetic digit width in pixels.
  constexpr int digitHalfHeight = 28;  // Defines half the synthetic digit height in pixels.
  std::vector<Color> pixels(size * size, backgroundColor);  // Declares pixels initialized to the synthetic background.
  for (int y = 0; y < size; ++y) {  // Iterates over each synthetic widget row.
    for (int x = 0; x < size; ++x) {  // Iterates over each synthetic widget column.
      const int dx = x - center;  // Sets dx to the horizontal distance from widget center.
      const int dy = y - center;  // Sets dy to the vertical distance from widget center.
      const int distanceSquared = dx * dx + dy * dy;  // Sets distanceSquared to the squared distance from widget center.
      const bool inRing = distanceSquared >= ringInnerSquared && distanceSquared <= ringOuterSquared;  // Sets inRing when the pixel belongs to the expected gear circle.
      const bool inDigit = dx >= -digitHalfWidth && dx <= digitHalfWidth && dy >= -digitHalfHeight && dy <= digitHalfHeight;  // Sets inDigit when the pixel belongs to the expected gear glyph.
      if (inRing || inDigit) {  // Guards the widget paint operation behind expected ring or glyph membership.
        pixels[static_cast<std::size_t>(y * size + x)] = widgetColor;  // Paints the synthetic widget pixel with the requested gear color.
      }  // Ends the current code block.
    }  // Ends the current code block.
  }  // Ends the current code block.
  return FrameRegion(Rect{0, 0, size, size}, std::move(pixels));  // Returns the synthetic widget region to the caller.
}  // Ends the current code block.

FrameRegion whiteWidgetOverRedMaskRegion() {  // Begins function whiteWidgetOverRedMaskRegion.
  constexpr int size = 160;  // Defines compile-time constant size as 160.
  constexpr int center = size / 2;  // Defines compile-time constant center as size / 2.
  constexpr int ringInnerSquared = 36 * 36;  // Defines the squared inner radius of the synthetic red scenery ring.
  constexpr int ringOuterSquared = 50 * 50;  // Defines the squared outer radius of the synthetic red scenery ring.
  constexpr int digitHalfWidth = 4;  // Defines half the synthetic white digit width in pixels.
  constexpr int digitHalfHeight = 28;  // Defines half the synthetic white digit height in pixels.
  std::vector<Color> pixels(size * size, Color{10, 10, 10, 255});  // Declares pixels initialized to a dark non-matching background.
  for (int y = 0; y < size; ++y) {  // Iterates over each synthetic region row.
    for (int x = 0; x < size; ++x) {  // Iterates over each synthetic region column.
      const int dx = x - center;  // Sets dx to the horizontal distance from widget center.
      const int dy = y - center;  // Sets dy to the vertical distance from widget center.
      const int distanceSquared = dx * dx + dy * dy;  // Sets distanceSquared to the squared distance from widget center.
      const bool inClassifierRedZone = distanceSquared <= ringOuterSquared &&  // Starts the red scenery mask condition for the classifier center and ring zones.
                                       (distanceSquared <= 32 * 32 || distanceSquared >= ringInnerSquared);  // Finishes the red scenery mask condition around the expected gear areas.
      if (inClassifierRedZone) {  // Guards red scenery painting behind the classifier zones that can create false positives.
        pixels[static_cast<std::size_t>(y * size + x)] = Color{216, 26, 52, 255};  // Paints red scenery under the HUD gear location.
      }  // Ends the current code block.
    }  // Ends the current code block.
  }  // Ends the current code block.
  for (int y = center - digitHalfHeight; y <= center + digitHalfHeight; ++y) {  // Iterates over the synthetic white gear digit rows.
    for (int x = center - digitHalfWidth; x <= center + digitHalfWidth; ++x) {  // Iterates over the synthetic white gear digit columns.
      pixels[static_cast<std::size_t>(y * size + x)] = Color{235, 235, 230, 255};  // Paints the visible white gear glyph over the red scenery.
    }  // Ends the current code block.
  }  // Ends the current code block.
  return FrameRegion(Rect{0, 0, size, size}, std::move(pixels));  // Returns the synthetic false-positive region to the caller.
}  // Ends the current code block.

}  // Ends the current code block.

FH6_TEST(color_classifier_detects_white) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_white).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = widgetRegion(Color{235, 235, 230, 255});  // Sets auto region to a white synthetic gear widget.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::White);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::White).
}  // Ends the current code block.

FH6_TEST(color_classifier_detects_red) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_red).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = widgetRegion(Color{240, 45, 30, 255});  // Sets auto region to a red synthetic gear widget.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::Red).
}  // Ends the current code block.

FH6_TEST(color_classifier_detects_fh6_shift_red) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_fh6_shift_red).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = widgetRegion(Color{216, 26, 52, 255});  // Sets auto region to a synthetic gear widget using the measured FH6 red.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::Red).
}  // Ends the current code block.

FH6_TEST(color_classifier_detects_glowing_fh_shift_red) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_glowing_fh_shift_red).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = widgetRegion(Color{214, 93, 159, 255});  // Sets auto region to a synthetic gear widget using a glowing FH6 red sample.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::Red).
}  // Ends the current code block.

FH6_TEST(color_classifier_rejects_sparse_red_without_gear_ring) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_rejects_sparse_red_without_gear_ring).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = sparseRegionWithGlyph(Color{214, 93, 159, 255});  // Sets auto region to sparseRegionWithGlyph(Color{214, 93, 159, 255}).

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Unknown);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::Unknown).
}  // Ends the current code block.

FH6_TEST(color_classifier_rejects_broad_red_background) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_rejects_broad_red_background).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = regionWithColor(Color{216, 26, 52, 255});  // Sets auto region to a solid red area matching the FH6 shift color.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Unknown);  // Requires broad red background to be rejected as a non-gear signal.
}  // Ends the current code block.

FH6_TEST(color_classifier_detects_white_widget_over_red_background) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_white_widget_over_red_background).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = widgetRegion(Color{235, 235, 230, 255}, Color{216, 26, 52, 255});  // Sets auto region to a white widget drawn over red scenery.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::White);  // Requires the widget color to win over the red background.
}  // Ends the current code block.

FH6_TEST(color_classifier_prefers_visible_white_gear_over_red_scenery) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_prefers_visible_white_gear_over_red_scenery).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = whiteWidgetOverRedMaskRegion();  // Sets auto region to a false-positive red scenery sample with a visible white gear.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::White);  // Requires visible white gear evidence to beat red scenery evidence.
}  // Ends the current code block.

FH6_TEST(color_classifier_detects_sparse_white_glyph) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_sparse_white_glyph).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = sparseRegionWithGlyph(Color{235, 235, 230, 255});  // Sets auto region to sparseRegionWithGlyph(Color{235, 235, 230, 255}).

  FH6_REQUIRE(classifier.classify(region) == GearColorState::White);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::White).
}  // Ends the current code block.

FH6_TEST(color_classifier_returns_unknown_for_ambiguous_region) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_returns_unknown_for_ambiguous_region).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = regionWithColor(Color{90, 90, 90, 255});  // Sets auto region to regionWithColor(Color{90, 90, 90, 255}).

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Unknown);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::Unknown).
}  // Ends the current code block.
