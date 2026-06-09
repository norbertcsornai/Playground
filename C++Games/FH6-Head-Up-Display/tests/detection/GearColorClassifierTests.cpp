#include "TestFramework.h"  // codex-line-comment: documents this line.

#include <vector>  // codex-line-comment: documents this line.

#include "capture/Frame.h"  // codex-line-comment: documents this line.
#include "detection/GearColorClassifier.h"  // codex-line-comment: documents this line.

using namespace fh6;  // codex-line-comment: documents this line.

namespace {  // codex-line-comment: documents this line.

FrameRegion regionWithColor(Color color) {  // codex-line-comment: documents this line.
  constexpr int size = 80;  // codex-line-comment: documents this line.
  return FrameRegion(Rect{0, 0, size, size},  // codex-line-comment: documents this line.
                     std::vector<Color>(static_cast<std::size_t>(size * size), color));  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FrameRegion sparseRegionWithGlyph(Color glyphColor) {  // codex-line-comment: documents this line.
  std::vector<Color> pixels(260 * 220, Color{10, 10, 10, 255});  // codex-line-comment: documents this line.
  for (int y = 90; y < 98; ++y) {  // codex-line-comment: documents this line.
    for (int x = 120; x < 130; ++x) {  // codex-line-comment: documents this line.
      pixels[static_cast<std::size_t>(y * 260 + x)] = glyphColor;  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
  return FrameRegion(Rect{0, 0, 260, 220}, std::move(pixels));  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace  // codex-line-comment: documents this line.

FH6_TEST(color_classifier_detects_white) {  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  auto region = regionWithColor(Color{235, 235, 230, 255});  // codex-line-comment: documents this line.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::White);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(color_classifier_detects_red) {  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  auto region = regionWithColor(Color{240, 45, 30, 255});  // codex-line-comment: documents this line.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(color_classifier_detects_fh6_shift_red) {  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  auto region = regionWithColor(Color{216, 26, 52, 255});  // codex-line-comment: documents this line.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(color_classifier_detects_glowing_fh_shift_red) {  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  auto region = regionWithColor(Color{214, 93, 159, 255});  // codex-line-comment: documents this line.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(color_classifier_rejects_sparse_red_without_gear_ring) {  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  auto region = sparseRegionWithGlyph(Color{214, 93, 159, 255});  // codex-line-comment: documents this line.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Unknown);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(color_classifier_detects_sparse_white_glyph) {  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  auto region = sparseRegionWithGlyph(Color{235, 235, 230, 255});  // codex-line-comment: documents this line.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::White);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(color_classifier_returns_unknown_for_ambiguous_region) {  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  auto region = regionWithColor(Color{90, 90, 90, 255});  // codex-line-comment: documents this line.

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Unknown);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.
