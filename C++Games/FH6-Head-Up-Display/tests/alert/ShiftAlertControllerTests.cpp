#include "TestFramework.h"  // codex-line-comment: documents this line.

#include "alert/ShiftAlertController.h"  // codex-line-comment: documents this line.

using namespace fh6;  // codex-line-comment: documents this line.

namespace {  // codex-line-comment: documents this line.

GearDetectionResult result(GearColorState state) {  // codex-line-comment: documents this line.
  return GearDetectionResult{GearValue::Unknown, state, 1.0F, Rect{0, 0, 10, 10}};  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace  // codex-line-comment: documents this line.

FH6_TEST(shift_alert_triggers_on_white_to_red) {  // codex-line-comment: documents this line.
  ShiftAlertController controller(std::chrono::milliseconds(100));  // codex-line-comment: documents this line.
  const auto now = Clock::now();  // codex-line-comment: documents this line.

  auto first = controller.update(result(GearColorState::White), now);  // codex-line-comment: documents this line.
  auto second = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // codex-line-comment: documents this line.

  FH6_REQUIRE(!first.triggeredThisFrame);  // codex-line-comment: documents this line.
  FH6_REQUIRE(second.triggeredThisFrame);  // codex-line-comment: documents this line.
  FH6_REQUIRE(second.active);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(shift_alert_does_not_retrigger_while_red) {  // codex-line-comment: documents this line.
  ShiftAlertController controller(std::chrono::milliseconds(100));  // codex-line-comment: documents this line.
  const auto now = Clock::now();  // codex-line-comment: documents this line.

  (void)controller.update(result(GearColorState::White), now);  // codex-line-comment: documents this line.
  auto firstRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // codex-line-comment: documents this line.
  auto secondRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(20));  // codex-line-comment: documents this line.

  FH6_REQUIRE(firstRed.triggeredThisFrame);  // codex-line-comment: documents this line.
  FH6_REQUIRE(!secondRed.triggeredThisFrame);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(shift_alert_resets_after_return_to_white) {  // codex-line-comment: documents this line.
  ShiftAlertController controller(std::chrono::milliseconds(100));  // codex-line-comment: documents this line.
  const auto now = Clock::now();  // codex-line-comment: documents this line.

  (void)controller.update(result(GearColorState::White), now);  // codex-line-comment: documents this line.
  (void)controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // codex-line-comment: documents this line.
  (void)controller.update(result(GearColorState::White), now + std::chrono::milliseconds(150));  // codex-line-comment: documents this line.
  auto secondShift = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(160));  // codex-line-comment: documents this line.

  FH6_REQUIRE(secondShift.triggeredThisFrame);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(shift_alert_triggers_when_first_confirmed_state_is_red) {  // codex-line-comment: documents this line.
  ShiftAlertController controller(std::chrono::milliseconds(100));  // codex-line-comment: documents this line.
  const auto now = Clock::now();  // codex-line-comment: documents this line.

  auto firstRed = controller.update(result(GearColorState::Red), now);  // codex-line-comment: documents this line.
  auto secondRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // codex-line-comment: documents this line.

  FH6_REQUIRE(firstRed.triggeredThisFrame);  // codex-line-comment: documents this line.
  FH6_REQUIRE(firstRed.active);  // codex-line-comment: documents this line.
  FH6_REQUIRE(!secondRed.triggeredThisFrame);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(shift_alert_remains_active_while_current_red_continues) {  // codex-line-comment: documents this line.
  ShiftAlertController controller(std::chrono::milliseconds(100));  // codex-line-comment: documents this line.
  const auto now = Clock::now();  // codex-line-comment: documents this line.

  (void)controller.update(result(GearColorState::White), now);  // codex-line-comment: documents this line.
  auto firstRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));  // codex-line-comment: documents this line.
  auto continuingRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(80));  // codex-line-comment: documents this line.

  FH6_REQUIRE(firstRed.triggeredThisFrame);  // codex-line-comment: documents this line.
  FH6_REQUIRE(continuingRed.active);  // codex-line-comment: documents this line.
  FH6_REQUIRE(!continuingRed.triggeredThisFrame);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

FH6_TEST(shift_alert_hides_immediately_when_gear_returns_white) {  // codex-line-comment: documents this line.
  ShiftAlertController controller(std::chrono::milliseconds(1000));  // codex-line-comment: documents this line.
  const auto now = Clock::now();  // codex-line-comment: documents this line.

  auto red = controller.update(result(GearColorState::Red), now);  // codex-line-comment: documents this line.
  auto white = controller.update(result(GearColorState::White), now + std::chrono::milliseconds(50));  // codex-line-comment: documents this line.

  FH6_REQUIRE(red.active);  // codex-line-comment: documents this line.
  FH6_REQUIRE(!white.active);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.
