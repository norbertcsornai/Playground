#include "TestFramework.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

#include "capture/Frame.h"
#include "detection/GearColorClassifier.h"
#include "detection/GearDetector.h"

using namespace fh6;

namespace {

void drawGearWidget(std::vector<Color>& pixels, int frameWidth, int centerX, int centerY,
                    Color color, int ringInnerRadius, int ringOuterRadius, int digitRadius) {
  const int maxRadius = std::max(ringOuterRadius, digitRadius);
  for (int y = centerY - maxRadius; y <= centerY + maxRadius; ++y) {
    for (int x = centerX - maxRadius; x <= centerX + maxRadius; ++x) {
      const int dx = x - centerX;
      const int dy = y - centerY;
      const int distanceSquared = dx * dx + dy * dy;
      const bool inRing = distanceSquared >= ringInnerRadius * ringInnerRadius &&
                          distanceSquared <= ringOuterRadius * ringOuterRadius;
      const bool inDigit = distanceSquared <= digitRadius * digitRadius && std::abs(dx) <= 2;
      if (inRing || inDigit) {
        pixels[static_cast<std::size_t>(y * frameWidth + x)] = color;
      }
    }
  }
}

}  // namespace

FH6_TEST(gear_detector_classifies_configured_region_color) {
  std::vector<Color> pixels(100 * 100, Color{20, 20, 20, 255});
  drawGearWidget(pixels, 100, 55, 57, Color{240, 45, 30, 255}, 9, 12, 7);

  Frame frame(100, 100, std::move(pixels));
  GearDetector detector;
  GearColorClassifier classifier;
  detector.setRegion(Rect{0, 0, 100, 100});

  const auto cropped = frame.crop(Rect{37, 39, 37, 37});
  FH6_REQUIRE(cropped.has_value());
  FH6_REQUIRE(classifier.classify(*cropped) == GearColorState::Red);

  const auto result = detector.detectGear(frame, classifier);

  FH6_REQUIRE(result.colorState == GearColorState::Red);
  FH6_REQUIRE(result.isConfident());
  FH6_REQUIRE(result.region.x == 37);
  FH6_REQUIRE(result.region.y == 39);
}

FH6_TEST(gear_detector_ignores_white_speed_text_outside_gear_focus_area) {
  std::vector<Color> pixels(420 * 420, Color{20, 20, 20, 255});

  for (int y = 300; y < 380; ++y) {
    for (int x = 240; x < 360; ++x) {
      pixels[static_cast<std::size_t>(y * 420 + x)] = Color{245, 245, 245, 255};
    }
  }

  drawGearWidget(pixels, 420, 232, 240, Color{214, 93, 159, 255}, 36, 50, 28);

  Frame frame(420, 420, std::move(pixels));
  GearDetector detector;
  GearColorClassifier classifier;
  detector.setRegion(Rect{0, 0, 420, 420});

  const auto result = detector.detectGear(frame, classifier);

  FH6_REQUIRE(result.colorState == GearColorState::Red);
}

FH6_TEST(gear_detector_ignores_red_tach_arc_outside_gear_focus_area) {
  std::vector<Color> pixels(420 * 420, Color{20, 20, 20, 255});

  for (int y = 110; y < 130; ++y) {
    for (int x = 330; x < 360; ++x) {
      pixels[static_cast<std::size_t>(y * 420 + x)] = Color{216, 26, 52, 255};
    }
  }

  drawGearWidget(pixels, 420, 232, 240, Color{235, 235, 230, 255}, 36, 50, 28);

  Frame frame(420, 420, std::move(pixels));
  GearDetector detector;
  GearColorClassifier classifier;
  detector.setRegion(Rect{0, 0, 420, 420});

  const auto result = detector.detectGear(frame, classifier);

  FH6_REQUIRE(result.colorState == GearColorState::White);
}
