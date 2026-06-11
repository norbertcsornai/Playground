#include "TestFramework.h"  // Imports project declarations from TestFramework.h.

#include <algorithm>  // Imports the algorithm standard library declarations used in this file.
#include <cstdlib>  // Imports the cstdlib standard library declarations used in this file.
#include <vector>  // Imports the vector standard library declarations used in this file.

#include "capture/Frame.h"  // Imports project declarations from capture/Frame.h.
#include "detection/GearColorClassifier.h"  // Imports project declarations from detection/GearColorClassifier.h.
#include "detection/GearDetector.h"  // Imports project declarations from detection/GearDetector.h.

using namespace fh6;  // Declares fh6 for use in this scope.

namespace {  // Starts a file-local helper namespace.

void drawGearWidget(std::vector<Color>& pixels, int frameWidth, int centerX, int centerY,  // Supplies void drawGearWidget(std::vector<Color>& pixels, int frameWidth, int centerX, int centerY to the surrounding call or initializer.
                    Color color, int ringInnerRadius, int ringOuterRadius, int digitRadius) {  // Starts a multi-line initializer or scope for Color color, int ringInnerRadius, int ringOuterRadius, int digitRadius).
  const int maxRadius = std::max(ringOuterRadius, digitRadius);  // Sets const int maxRadius to std::max(ringOuterRadius, digitRadius).
  for (int y = centerY - maxRadius; y <= centerY + maxRadius; ++y) {  // Iterates with loop control int y = centerY - maxRadius; y <= centerY + maxRadius; ++y.
    for (int x = centerX - maxRadius; x <= centerX + maxRadius; ++x) {  // Iterates with loop control int x = centerX - maxRadius; x <= centerX + maxRadius; ++x.
      const int dx = x - centerX;  // Sets const int dx to x - centerX.
      const int dy = y - centerY;  // Sets const int dy to y - centerY.
      const int distanceSquared = dx * dx + dy * dy;  // Sets const int distanceSquared to dx * dx + dy * dy.
      const bool inRing = distanceSquared >= ringInnerRadius * ringInnerRadius &&  // Continues the surrounding declaration or control-flow expression.
                          distanceSquared <= ringOuterRadius * ringOuterRadius;  // Sets distanceSquared < to ringOuterRadius * ringOuterRadius.
      const bool inDigit = distanceSquared <= digitRadius * digitRadius && std::abs(dx) <= 2;  // Sets const bool inDigit to distanceSquared <= digitRadius * digitRadius && std::abs(dx) <= 2.
      if (inRing || inDigit) {  // Guards the following work behind the condition inRing || inDigit.
        pixels[static_cast<std::size_t>(y * frameWidth + x)] = color;  // Sets pixels[static_cast<std::size_t>(y * frameWidth + x)] to color.
      }  // Ends the current code block.
    }  // Ends the current code block.
  }  // Ends the current code block.
}  // Ends the current code block.

}  // Ends the current code block.

FH6_TEST(gear_detector_classifies_configured_region_color) {  // Starts a multi-line initializer or scope for FH6_TEST(gear_detector_classifies_configured_region_color).
  std::vector<Color> pixels(100 * 100, Color{20, 20, 20, 255});  // Declares function pixels for callers.
  drawGearWidget(pixels, 100, 55, 57, Color{240, 45, 30, 255}, 9, 12, 7);  // Invokes drawGearWidget with the supplied arguments.

  Frame frame(100, 100, std::move(pixels));  // Declares function frame for callers.
  GearDetector detector;  // Declares detector for use in this scope.
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  detector.setRegion(Rect{0, 0, 100, 100});  // Calls setRegion on detector.

  const auto cropped = frame.crop(Rect{37, 39, 37, 37});  // Sets const auto cropped to frame.crop(Rect{37, 39, 37, 37}).
  FH6_REQUIRE(cropped.has_value());  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(classifier.classify(*cropped) == GearColorState::Red);  // Sets FH6_REQUIRE(classifier.classify(*cropped) to = GearColorState::Red).

  const auto result = detector.detectGear(frame, classifier);  // Sets const auto result to detector.detectGear(frame, classifier).

  FH6_REQUIRE(result.colorState == GearColorState::Red);  // Sets FH6_REQUIRE(result.colorState to = GearColorState::Red).
  FH6_REQUIRE(result.isConfident());  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(result.region.x == 37);  // Sets FH6_REQUIRE(result.region.x to = 37).
  FH6_REQUIRE(result.region.y == 39);  // Sets FH6_REQUIRE(result.region.y to = 39).
}  // Ends the current code block.

FH6_TEST(gear_detector_ignores_white_speed_text_outside_gear_focus_area) {  // Starts a multi-line initializer or scope for FH6_TEST(gear_detector_ignores_white_speed_text_outside_gear_focus_area).
  std::vector<Color> pixels(420 * 420, Color{20, 20, 20, 255});  // Declares function pixels for callers.

  for (int y = 300; y < 380; ++y) {  // Iterates with loop control int y = 300; y < 380; ++y.
    for (int x = 240; x < 360; ++x) {  // Iterates with loop control int x = 240; x < 360; ++x.
      pixels[static_cast<std::size_t>(y * 420 + x)] = Color{245, 245, 245, 255};  // Sets pixels[static_cast<std::size_t>(y * 420 + x)] to Color{245, 245, 245, 255}.
    }  // Ends the current code block.
  }  // Ends the current code block.

  drawGearWidget(pixels, 420, 232, 240, Color{214, 93, 159, 255}, 36, 50, 28);  // Invokes drawGearWidget with the supplied arguments.

  Frame frame(420, 420, std::move(pixels));  // Declares function frame for callers.
  GearDetector detector;  // Declares detector for use in this scope.
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  detector.setRegion(Rect{0, 0, 420, 420});  // Calls setRegion on detector.

  const auto result = detector.detectGear(frame, classifier);  // Sets const auto result to detector.detectGear(frame, classifier).

  FH6_REQUIRE(result.colorState == GearColorState::Red);  // Sets FH6_REQUIRE(result.colorState to = GearColorState::Red).
}  // Ends the current code block.

FH6_TEST(gear_detector_ignores_red_tach_arc_outside_gear_focus_area) {  // Starts a multi-line initializer or scope for FH6_TEST(gear_detector_ignores_red_tach_arc_outside_gear_focus_area).
  std::vector<Color> pixels(420 * 420, Color{20, 20, 20, 255});  // Declares function pixels for callers.

  for (int y = 110; y < 130; ++y) {  // Iterates with loop control int y = 110; y < 130; ++y.
    for (int x = 330; x < 360; ++x) {  // Iterates with loop control int x = 330; x < 360; ++x.
      pixels[static_cast<std::size_t>(y * 420 + x)] = Color{216, 26, 52, 255};  // Sets pixels[static_cast<std::size_t>(y * 420 + x)] to Color{216, 26, 52, 255}.
    }  // Ends the current code block.
  }  // Ends the current code block.

  drawGearWidget(pixels, 420, 232, 240, Color{235, 235, 230, 255}, 36, 50, 28);  // Invokes drawGearWidget with the supplied arguments.

  Frame frame(420, 420, std::move(pixels));  // Declares function frame for callers.
  GearDetector detector;  // Declares detector for use in this scope.
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  detector.setRegion(Rect{0, 0, 420, 420});  // Calls setRegion on detector.

  const auto result = detector.detectGear(frame, classifier);  // Sets const auto result to detector.detectGear(frame, classifier).

  FH6_REQUIRE(result.colorState == GearColorState::White);  // Sets FH6_REQUIRE(result.colorState to = GearColorState::White).
}  // Ends the current code block.
