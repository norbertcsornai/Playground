#include "TestFramework.h"  // Imports project declarations from TestFramework.h.

#include "overlay/OverlayWindow.h"  // Imports project declarations from overlay/OverlayWindow.h.
#include "shared/DisplayInfo.h"  // Imports project declarations from shared/DisplayInfo.h.

using namespace fh6;  // Places fh6 symbols into this test translation unit.

// showArrow() and hideArrow() return early when the overlay is already in the requested state, so
// that a loop calling them every pass does not hammer ShowWindow. These cover the risk in that
// optimisation: a visibility flag left out of step would stop the cue appearing at all.
FH6_TEST(overlay_show_and_hide_are_repeatable_and_reversible) {  // Starts a multi-line initializer or scope for overlay_show_and_hide_are_repeatable_and_reversible.
  OverlayWindow overlay;  // Declares overlay for use in this scope.
  const DisplayInfo display{"test", Rect{0, 0, 800, 600}, 1.0F, false};  // Declares a synthetic display to place the overlay on.

  if (!overlay.create(display)) {  // Guards the following work behind successful window creation.
    return;  // Skips this test where no interactive window station is available.
  }  // Ends the current code block.

  FH6_REQUIRE(overlay.isCreated());  // Requires the overlay window to exist after creation.
  FH6_REQUIRE(!overlay.isVisible());  // Requires a freshly created overlay to start hidden.

  overlay.showArrow();  // Calls showArrow on overlay.
  FH6_REQUIRE(overlay.isVisible());  // Requires the first show to make the arrow visible.

  overlay.showArrow();  // Repeats the show to exercise the already-visible early return.
  FH6_REQUIRE(overlay.isVisible());  // Requires a repeated show to leave the arrow visible.

  overlay.hideArrow();  // Calls hideArrow on overlay.
  FH6_REQUIRE(!overlay.isVisible());  // Requires the hide to take effect.

  overlay.hideArrow();  // Repeats the hide to exercise the already-hidden early return.
  FH6_REQUIRE(!overlay.isVisible());  // Requires a repeated hide to leave the arrow hidden.

  // The important regression: a later alert must still be able to show the arrow again.
  overlay.showArrow();  // Calls showArrow on overlay after it was hidden.
  FH6_REQUIRE(overlay.isVisible());  // Requires the overlay to become visible again.

  overlay.destroy();  // Calls destroy on overlay.
  FH6_REQUIRE(!overlay.isCreated());  // Requires destroy to release the window.
  FH6_REQUIRE(!overlay.isVisible());  // Requires destroy to leave the overlay hidden.
}  // Ends the current code block.

// The stutter fix: toggling the arrow must never toggle the window itself. Showing or hiding a
// topmost layered window makes the compositor re-evaluate the fullscreen game underneath it, which
// costs the game frames every time the gear flickers between white and red.
FH6_TEST(overlay_toggling_arrow_never_toggles_window_visibility) {  // Starts a multi-line initializer or scope for overlay_toggling_arrow_never_toggles_window_visibility.
  OverlayWindow overlay;  // Declares overlay for use in this scope.
  const DisplayInfo display{"test", Rect{0, 0, 800, 600}, 1.0F, false};  // Declares a synthetic display to place the overlay on.

  if (!overlay.create(display)) {  // Guards the following work behind successful window creation.
    return;  // Skips this test where no interactive window station is available.
  }  // Ends the current code block.

  FH6_REQUIRE(overlay.isWindowShown());  // Requires the window to be shown from creation onwards.
  FH6_REQUIRE(!overlay.isVisible());  // Requires the arrow itself to start hidden.

  // Drive the white -> red -> white flicker that was disturbing the game.
  for (int cycle = 0; cycle < 5; ++cycle) {  // Iterates over several show and hide cycles.
    overlay.showArrow();  // Calls showArrow on overlay.
    FH6_REQUIRE(overlay.isVisible());  // Requires the arrow to be painted.
    FH6_REQUIRE(overlay.isWindowShown());  // Requires showing the arrow to leave the window shown.

    overlay.hideArrow();  // Calls hideArrow on overlay.
    FH6_REQUIRE(!overlay.isVisible());  // Requires the arrow to be cleared.
    FH6_REQUIRE(overlay.isWindowShown());  // Requires hiding the arrow to leave the window shown.
  }  // Ends the current code block.

  overlay.destroy();  // Calls destroy on overlay.
}  // Ends the current code block.

FH6_TEST(overlay_recentering_same_display_keeps_arrow_visible) {  // Starts a multi-line initializer or scope for overlay_recentering_same_display_keeps_arrow_visible.
  OverlayWindow overlay;  // Declares overlay for use in this scope.
  const DisplayInfo display{"test", Rect{0, 0, 800, 600}, 1.0F, false};  // Declares a synthetic display to place the overlay on.

  if (!overlay.create(display)) {  // Guards the following work behind successful window creation.
    return;  // Skips this test where no interactive window station is available.
  }  // Ends the current code block.

  overlay.showArrow();  // Calls showArrow on overlay.
  FH6_REQUIRE(overlay.isVisible());  // Requires the arrow to be visible before recentering.

  // Recentering on the unchanged display skips SetWindowPos; it must not disturb visibility.
  overlay.centerOnDisplay(display);  // Calls centerOnDisplay on overlay.
  FH6_REQUIRE(overlay.isVisible());  // Requires the arrow to stay visible after a no-op recenter.

  overlay.destroy();  // Calls destroy on overlay.
}  // Ends the current code block.
