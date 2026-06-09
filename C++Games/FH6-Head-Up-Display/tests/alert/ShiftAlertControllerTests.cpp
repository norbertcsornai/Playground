#include "TestFramework.h"

#include "alert/ShiftAlertController.h"

using namespace fh6;

namespace {

GearDetectionResult result(GearColorState state) {
  return GearDetectionResult{GearValue::Unknown, state, 1.0F, Rect{0, 0, 10, 10}};
}

}  // namespace

FH6_TEST(shift_alert_triggers_on_white_to_red) {
  ShiftAlertController controller(std::chrono::milliseconds(100));
  const auto now = Clock::now();

  auto first = controller.update(result(GearColorState::White), now);
  auto second = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));

  FH6_REQUIRE(!first.triggeredThisFrame);
  FH6_REQUIRE(second.triggeredThisFrame);
  FH6_REQUIRE(second.active);
}

FH6_TEST(shift_alert_does_not_retrigger_while_red) {
  ShiftAlertController controller(std::chrono::milliseconds(100));
  const auto now = Clock::now();

  (void)controller.update(result(GearColorState::White), now);
  auto firstRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));
  auto secondRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(20));

  FH6_REQUIRE(firstRed.triggeredThisFrame);
  FH6_REQUIRE(!secondRed.triggeredThisFrame);
}

FH6_TEST(shift_alert_resets_after_return_to_white) {
  ShiftAlertController controller(std::chrono::milliseconds(100));
  const auto now = Clock::now();

  (void)controller.update(result(GearColorState::White), now);
  (void)controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));
  (void)controller.update(result(GearColorState::White), now + std::chrono::milliseconds(150));
  auto secondShift = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(160));

  FH6_REQUIRE(secondShift.triggeredThisFrame);
}

FH6_TEST(shift_alert_triggers_when_first_confirmed_state_is_red) {
  ShiftAlertController controller(std::chrono::milliseconds(100));
  const auto now = Clock::now();

  auto firstRed = controller.update(result(GearColorState::Red), now);
  auto secondRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));

  FH6_REQUIRE(firstRed.triggeredThisFrame);
  FH6_REQUIRE(firstRed.active);
  FH6_REQUIRE(!secondRed.triggeredThisFrame);
}

FH6_TEST(shift_alert_remains_active_while_current_red_continues) {
  ShiftAlertController controller(std::chrono::milliseconds(100));
  const auto now = Clock::now();

  (void)controller.update(result(GearColorState::White), now);
  auto firstRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(10));
  auto continuingRed = controller.update(result(GearColorState::Red), now + std::chrono::milliseconds(80));

  FH6_REQUIRE(firstRed.triggeredThisFrame);
  FH6_REQUIRE(continuingRed.active);
  FH6_REQUIRE(!continuingRed.triggeredThisFrame);
}

FH6_TEST(shift_alert_hides_immediately_when_gear_returns_white) {
  ShiftAlertController controller(std::chrono::milliseconds(1000));
  const auto now = Clock::now();

  auto red = controller.update(result(GearColorState::Red), now);
  auto white = controller.update(result(GearColorState::White), now + std::chrono::milliseconds(50));

  FH6_REQUIRE(red.active);
  FH6_REQUIRE(!white.active);
}
