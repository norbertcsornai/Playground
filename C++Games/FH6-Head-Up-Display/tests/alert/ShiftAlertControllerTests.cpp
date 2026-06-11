#include "TestFramework.h"  // Imports project declarations from TestFramework.h.

#include "alert/ShiftAlertController.h"  // Imports project declarations from alert/ShiftAlertController.h.

using namespace fh6;  // Declares fh6 for use in this scope.

namespace {  // Starts a file-local helper namespace.

GearDetectionResult result(GearColorState state) {  // Begins function result.
  return GearDetectionResult{GearValue::Unknown, state, 1.0F, Rect{0, 0, 10, 10}};  // Returns GearDetectionResult{GearValue::Unknown, state, 1.0F, Rect{0, 0, 10, 10}} to the caller.
}  // Ends the current code block.

}  // Ends the current code block.

FH6_TEST(shift_alert_triggers_on_white_to_red) {  // Starts a multi-line initializer or scope for FH6_TEST(shift_alert_triggers_on_white_to_red).
  ShiftAlertController controller(std::chrono::milliseconds(100));  // Declares function controller for callers.
  const auto now = Clock::now();  // Sets const auto now to Clock::now().

  auto first = controller.update(result(GearColorState::White), now);  // Sets auto first to controller.update(result(GearColorState::White), now).
  auto second = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // Sets auto second to controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10)).

  FH6_REQUIRE(!first.triggeredThisFrame);  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(second.triggeredThisFrame);  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(second.active);  // Invokes FH6_REQUIRE with the supplied arguments.
}  // Ends the current code block.

FH6_TEST(shift_alert_does_not_retrigger_while_red) {  // Starts a multi-line initializer or scope for FH6_TEST(shift_alert_does_not_retrigger_while_red).
  ShiftAlertController controller(std::chrono::milliseconds(100));  // Declares function controller for callers.
  const auto now = Clock::now();  // Sets const auto now to Clock::now().

  (void)controller.update(result(GearColorState::White), now);  // Executes (void)controller.update(result(GearColorState::White), now).
  auto firstRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // Sets auto firstRed to controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10)).
  auto secondRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(20));  // Sets auto secondRed to controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(20)).

  FH6_REQUIRE(firstRed.triggeredThisFrame);  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(!secondRed.triggeredThisFrame);  // Invokes FH6_REQUIRE with the supplied arguments.
}  // Ends the current code block.

FH6_TEST(shift_alert_resets_after_return_to_white) {  // Starts a multi-line initializer or scope for FH6_TEST(shift_alert_resets_after_return_to_white).
  ShiftAlertController controller(std::chrono::milliseconds(100));  // Declares function controller for callers.
  const auto now = Clock::now();  // Sets const auto now to Clock::now().

  (void)controller.update(result(GearColorState::White), now);  // Executes (void)controller.update(result(GearColorState::White), now).
  (void)controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // Executes (void)controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10)).
  (void)controller.update(result(GearColorState::White), now + std::chrono::milliseconds(150));  // Executes (void)controller.update(result(GearColorState::White), now + std::chrono::milliseconds(....
  auto secondShift = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(160));  // Sets auto secondShift to controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(160)).

  FH6_REQUIRE(secondShift.triggeredThisFrame);  // Invokes FH6_REQUIRE with the supplied arguments.
}  // Ends the current code block.

FH6_TEST(shift_alert_triggers_when_first_confirmed_state_is_red) {  // Starts a multi-line initializer or scope for FH6_TEST(shift_alert_triggers_when_first_confirmed_state_is_red).
  ShiftAlertController controller(std::chrono::milliseconds(100));  // Declares function controller for callers.
  const auto now = Clock::now();  // Sets const auto now to Clock::now().

  auto firstRed = controller.update(result(GearColorState::Red), now);  // Sets auto firstRed to controller.update(result(GearColorState::Red), now).
  auto secondRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // Sets auto secondRed to controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10)).

  FH6_REQUIRE(firstRed.triggeredThisFrame);  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(firstRed.active);  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(!secondRed.triggeredThisFrame);  // Invokes FH6_REQUIRE with the supplied arguments.
}  // Ends the current code block.

FH6_TEST(shift_alert_remains_active_while_current_red_continues) {  // Starts a multi-line initializer or scope for FH6_TEST(shift_alert_remains_active_while_current_red_continues).
  ShiftAlertController controller(std::chrono::milliseconds(100));  // Declares function controller for callers.
  const auto now = Clock::now();  // Sets const auto now to Clock::now().

  (void)controller.update(result(GearColorState::White), now);  // Executes (void)controller.update(result(GearColorState::White), now).
  auto firstRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // Sets auto firstRed to controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10)).
  auto continuingRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(80));  // Sets auto continuingRed to controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(80)).

  FH6_REQUIRE(firstRed.triggeredThisFrame);  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(continuingRed.active);  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(!continuingRed.triggeredThisFrame);  // Invokes FH6_REQUIRE with the supplied arguments.
}  // Ends the current code block.

FH6_TEST(shift_alert_hides_immediately_when_gear_returns_white) {  // Starts a multi-line initializer or scope for FH6_TEST(shift_alert_hides_immediately_when_gear_returns_white).
  ShiftAlertController controller(std::chrono::milliseconds(1000));  // Declares function controller for callers.
  const auto now = Clock::now();  // Sets const auto now to Clock::now().

  auto red = controller.update(result(GearColorState::Red), now);  // Sets auto red to controller.update(result(GearColorState::Red), now).
  auto white = controller.update(result(GearColorState::White), now + std::chrono::milliseconds(50));  // Sets auto white to controller.update(result(GearColorState::White), now + std::chrono::milliseconds(50)).

  FH6_REQUIRE(red.active);  // Invokes FH6_REQUIRE with the supplied arguments.
  FH6_REQUIRE(!white.active);  // Invokes FH6_REQUIRE with the supplied arguments.
}  // Ends the current code block.
