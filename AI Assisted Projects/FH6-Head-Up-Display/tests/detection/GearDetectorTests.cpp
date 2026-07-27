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

// Mirrors what the app does in production: capture fetches only the HUD rect that
// gearRegionForDisplay() reports for the live display, and the detector must still locate the gear
// inside that sub-rectangle frame. This is the path DesktopFrameCapture's region-of-interest
// support produces, so a coordinate-mapping mistake between the two would surface here.
FH6_TEST(gear_detector_reads_gear_from_region_of_interest_frame) {  // Starts a multi-line initializer or scope for gear_detector_reads_gear_from_region_of_interest_frame.
  constexpr int kDisplayWidth = 1920;  // Defines compile-time constant kDisplayWidth as 1920.
  constexpr int kDisplayHeight = 1080;  // Defines compile-time constant kDisplayHeight as 1080.

  GearDetector detector;  // Declares detector for use in this scope.
  GearColorClassifier classifier;  // Declares classifier for use in this scope.
  detector.setRegion(Rect{-1, -1, 420, 420});  // Selects the dynamic lower-right default region.

  const Rect roi = detector.gearRegionForDisplay(kDisplayWidth, kDisplayHeight);  // Sets const Rect roi to the HUD rect the capture layer would fetch.
  FH6_REQUIRE(roi.width > 0 && roi.height > 0);  // Requires the reported region to be usable.

  // Paint the gear widget centered in the focus area the detector looks at inside that region,
  // using buffer-local coordinates (display coordinates minus the region origin).
  std::vector<Color> pixels(static_cast<std::size_t>(roi.width) * roi.height, Color{20, 20, 20, 255});  // Declares pixels initialized to a dark background.
  const int focusX = static_cast<int>(static_cast<double>(roi.width) * 0.37);  // Sets const int focusX to the focus area's left offset inside the region.
  const int focusY = static_cast<int>(static_cast<double>(roi.height) * 0.39);  // Sets const int focusY to the focus area's top offset inside the region.
  const int focusSize = static_cast<int>(static_cast<double>(roi.width) * 0.37);  // Sets const int focusSize to the focus area's side length.
  drawGearWidget(pixels, roi.width, focusX + focusSize / 2, focusY + focusSize / 2,  // Invokes drawGearWidget centered on the focus area.
                 Color{216, 26, 52, 255}, 36, 50, 28);  // Supplies the measured FH6 shift red and widget radii.

  const Frame frame(roi, kDisplayWidth, kDisplayHeight, std::move(pixels));  // Declares a frame holding only the captured HUD region.

  const auto result = detector.detectGear(frame, classifier);  // Sets const auto result to detector.detectGear(frame, classifier).

  FH6_REQUIRE(result.colorState == GearColorState::Red);  // Requires the gear to be found inside the region-of-interest frame.
  FH6_REQUIRE(result.isConfident());  // Requires the detection to be confident.
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
