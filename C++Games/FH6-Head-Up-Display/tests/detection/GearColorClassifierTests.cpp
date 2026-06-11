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

}  // Ends the current code block.

FH6_TEST(color_classifier_detects_white) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_white).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = regionWithColor(Color{235, 235, 230, 255});  // Sets auto region to regionWithColor(Color{235, 235, 230, 255}).

  FH6_REQUIRE(classifier.classify(region) == GearColorState::White);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::White).
}  // Ends the current code block.

FH6_TEST(color_classifier_detects_red) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_red).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = regionWithColor(Color{240, 45, 30, 255});  // Sets auto region to regionWithColor(Color{240, 45, 30, 255}).

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::Red).
}  // Ends the current code block.

FH6_TEST(color_classifier_detects_fh6_shift_red) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_fh6_shift_red).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = regionWithColor(Color{216, 26, 52, 255});  // Sets auto region to regionWithColor(Color{216, 26, 52, 255}).

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::Red).
}  // Ends the current code block.

FH6_TEST(color_classifier_detects_glowing_fh_shift_red) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_detects_glowing_fh_shift_red).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = regionWithColor(Color{214, 93, 159, 255});  // Sets auto region to regionWithColor(Color{214, 93, 159, 255}).

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::Red).
}  // Ends the current code block.

FH6_TEST(color_classifier_rejects_sparse_red_without_gear_ring) {  // Starts a multi-line initializer or scope for FH6_TEST(color_classifier_rejects_sparse_red_without_gear_ring).
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  auto region = sparseRegionWithGlyph(Color{214, 93, 159, 255});  // Sets auto region to sparseRegionWithGlyph(Color{214, 93, 159, 255}).

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Unknown);  // Sets FH6_REQUIRE(classifier.classify(region) to = GearColorState::Unknown).
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
