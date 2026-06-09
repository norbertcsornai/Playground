#include "TestFramework.h"

#include <vector>

#include "capture/Frame.h"
#include "detection/GearColorClassifier.h"

using namespace fh6;

namespace {

FrameRegion regionWithColor(Color color) {
  constexpr int size = 80;
  return FrameRegion(Rect{0, 0, size, size},
                     std::vector<Color>(static_cast<std::size_t>(size * size), color));
}

FrameRegion sparseRegionWithGlyph(Color glyphColor) {
  std::vector<Color> pixels(260 * 220, Color{10, 10, 10, 255});
  for (int y = 90; y < 98; ++y) {
    for (int x = 120; x < 130; ++x) {
      pixels[static_cast<std::size_t>(y * 260 + x)] = glyphColor;
    }
  }
  return FrameRegion(Rect{0, 0, 260, 220}, std::move(pixels));
}

}  // namespace

FH6_TEST(color_classifier_detects_white) {
  GearColorClassifier classifier;
  auto region = regionWithColor(Color{235, 235, 230, 255});

  FH6_REQUIRE(classifier.classify(region) == GearColorState::White);
}

FH6_TEST(color_classifier_detects_red) {
  GearColorClassifier classifier;
  auto region = regionWithColor(Color{240, 45, 30, 255});

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);
}

FH6_TEST(color_classifier_detects_fh6_shift_red) {
  GearColorClassifier classifier;
  auto region = regionWithColor(Color{216, 26, 52, 255});

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);
}

FH6_TEST(color_classifier_detects_glowing_fh_shift_red) {
  GearColorClassifier classifier;
  auto region = regionWithColor(Color{214, 93, 159, 255});

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Red);
}

FH6_TEST(color_classifier_rejects_sparse_red_without_gear_ring) {
  GearColorClassifier classifier;
  auto region = sparseRegionWithGlyph(Color{214, 93, 159, 255});

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Unknown);
}

FH6_TEST(color_classifier_detects_sparse_white_glyph) {
  GearColorClassifier classifier;
  auto region = sparseRegionWithGlyph(Color{235, 235, 230, 255});

  FH6_REQUIRE(classifier.classify(region) == GearColorState::White);
}

FH6_TEST(color_classifier_returns_unknown_for_ambiguous_region) {
  GearColorClassifier classifier;
  auto region = regionWithColor(Color{90, 90, 90, 255});

  FH6_REQUIRE(classifier.classify(region) == GearColorState::Unknown);
}
