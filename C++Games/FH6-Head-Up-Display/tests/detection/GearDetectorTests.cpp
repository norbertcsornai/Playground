#include "TestFramework.h"  // codex-line-comment: documents this line.

#include <algorithm>  // codex-line-comment: documents this line.
#include <cstdlib>  // codex-line-comment: documents this line.
#include <vector>  // codex-line-comment: documents this line.

#include "capture/Frame.h"  // codex-line-comment: documents this line.
#include "detection/GearColorClassifier.h"  // codex-line-comment: documents this line.
#include "detection/GearDetector.h"  // codex-line-comment: documents this line.

using namespace fh6;  // codex-line-comment: documents this line.

namespace {  // codex-line-comment: documents this line.

void drawGearWidget(std::vector<Color>& pixels, int frameWidth, int centerX, int centerY,  // codex-line-comment: documents this line.
                    Color color, int ringInnerRadius, int ringOuterRadius, int digitRadius) {  // codex-line-comment: documents this line.
  const int maxRadius = std::max(ringOuterRadius, digitRadius);  // codex-line-comment: documents this line.
  for (int y = centerY - maxRadius; y <= centerY + maxRadius; ++y) {  // codex-line-comment: documents this line.
    for (int x = centerX - maxRadius; x <= centerX + maxRadius; ++x) {  // codex-line-comment: documents this line.
      const int dx = x - centerX;  // codex-line-comment: documents this line.
      const int dy = y - centerY;  // codex-line-comment: documents this line.
      const int distanceSquared = dx * dx + dy * dy;  // codex-line-comment: documents this line.
      const bool inRing = distanceSquared >= ringInnerRadius * ringInnerRadius &&  // codex-line-comment: documents this line.
                          distanceSquared <= ringOuterRadius * ringOuterRadius;  // codex-line-comment: documents this line.
      const bool inDigit = distanceSquared <= digitRadius * digitRadius && std::abs(dx) <= 2;  // codex-line-comment: documents this line.
      if (inRing || inDigit) {  // codex-line-comment: documents this line.
        pixels[static_cast<std::size_t>(y * frameWidth + x)] = color;  // codex-line-comment: documents this line.
      }  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace  // codex-line-comment: documents this line.

FH6_TEST(gear_detector_classifies_configured_region_color) {  // codex-line-comment: documents this line.
  std::vector<Color> pixels(100 * 100, Color{20, 20, 20, 255});  // codex-line-comment: documents this line.
  drawGearWidget(pixels, 100, 55, 57, Color{240, 45, 30, 255}, 9, 12, 7);  // codex-line-comment: documents this line.

  Frame frame(100, 100, std::move(pixels));  // codex-line-comment: documents this line.
  GearDetector detector;  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  detector.setRegion(Rect{0, 0, 100, 100});  // codex-line-comment: documents this line.

  const auto cropped = frame.crop(Rect{37, 39, 37, 37});  // codex-line-comment: documents this line.
  FH6_REQUIRE(cropped.has_value());  // codex-line-comment: documents this line.
  FH6_REQUIRE(classifier.classify(*cropped) == GearColorState::Red);  // codex-line-comment: documents this line.

  const auto result = detector.detectGear(frame, classifier);  // codex-line-comment: documents this line.

  FH6_REQUIRE(result.colorState == GearColorState::Red);  // codex-line-comment: documents this line.
  FH6_REQUIRE(result.isConfident());  // codex-line-comment: documents this line.
  FH6_REQUIRE(result.region.x == 37);  // codex-line-comment: documents this line.
  FH6_REQUIRE(result.region.y == 39);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(gear_detector_ignores_white_speed_text_outside_gear_focus_area) {  // codex-line-comment: documents this line.
  std::vector<Color> pixels(420 * 420, Color{20, 20, 20, 255});  // codex-line-comment: documents this line.

  for (int y = 300; y < 380; ++y) {  // codex-line-comment: documents this line.
    for (int x = 240; x < 360; ++x) {  // codex-line-comment: documents this line.
      pixels[static_cast<std::size_t>(y * 420 + x)] = Color{245, 245, 245, 255};  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  drawGearWidget(pixels, 420, 232, 240, Color{214, 93, 159, 255}, 36, 50, 28);  // codex-line-comment: documents this line.

  Frame frame(420, 420, std::move(pixels));  // codex-line-comment: documents this line.
  GearDetector detector;  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  detector.setRegion(Rect{0, 0, 420, 420});  // codex-line-comment: documents this line.

  const auto result = detector.detectGear(frame, classifier);  // codex-line-comment: documents this line.

  FH6_REQUIRE(result.colorState == GearColorState::Red);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(gear_detector_ignores_red_tach_arc_outside_gear_focus_area) {  // codex-line-comment: documents this line.
  std::vector<Color> pixels(420 * 420, Color{20, 20, 20, 255});  // codex-line-comment: documents this line.

  for (int y = 110; y < 130; ++y) {  // codex-line-comment: documents this line.
    for (int x = 330; x < 360; ++x) {  // codex-line-comment: documents this line.
      pixels[static_cast<std::size_t>(y * 420 + x)] = Color{216, 26, 52, 255};  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  drawGearWidget(pixels, 420, 232, 240, Color{235, 235, 230, 255}, 36, 50, 28);  // codex-line-comment: documents this line.

  Frame frame(420, 420, std::move(pixels));  // codex-line-comment: documents this line.
  GearDetector detector;  // codex-line-comment: documents this line.
  GearColorClassifier classifier;  // codex-line-comment: documents this line.
  detector.setRegion(Rect{0, 0, 420, 420});  // codex-line-comment: documents this line.

  const auto result = detector.detectGear(frame, classifier);  // codex-line-comment: documents this line.

  FH6_REQUIRE(result.colorState == GearColorState::White);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.
