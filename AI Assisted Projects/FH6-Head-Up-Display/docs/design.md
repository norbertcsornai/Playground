# FH6 Head-Up Display Design

## Purpose

This document describes the proposed architecture for the FH6 Head-Up Display C++ application. It is based on the requirements in `docs/requirements.md` and the class model in `docs/uml.puml`.

The application runs alongside Forza Horizon 6, reads visible game frames, detects the current gear value and gear color, and displays a 50x100 pixel red arrow at the center of the active game display when the gear indicator transitions from white to red.

The design prioritizes low latency, non-disruptive behavior, game integrity, testability, and clear separation between capture, detection, alert logic, overlay rendering, configuration, and diagnostics.

## Architecture

The application is organized as a small real-time processing pipeline coordinated by `HudApplication`.

At a high level:

1. `GameWindowTracker` detects whether Forza Horizon 6 is running and visible.
2. `DesktopFrameCapture` acquires frames from the display containing the game.
3. `GearDetector` extracts the configured gear HUD region and recognizes the gear value.
4. `GearColorClassifier` classifies the gear indicator color as white, red, or unknown.
5. `ShiftAlertController` detects confirmed white-to-red transitions.
6. `OverlayWindow` displays or hides the centered red arrow.
7. `ConfigStore` loads and saves user configuration.
8. `DiagnosticsService` records status and troubleshooting information.

The system does not modify game files, read or write game memory, inject inputs, or hook game input APIs. It relies on normal Windows desktop capture and transparent overlay behavior.

## Component Responsibilities

### HudApplication

`HudApplication` is the application orchestrator. It owns the main lifecycle and coordinates all major components.

Responsibilities:

- Load configuration on startup.
- Initialize game tracking, frame capture, detection, alert logic, overlay, and diagnostics.
- Run the main processing loop.
- Enter idle mode when the game is missing, minimized, or unavailable for capture.
- Recover when the game starts, exits, alt-tabs, minimizes, or changes display mode.
- Shut down components cleanly.

Key methods:

- `initialize()`
- `run()`
- `shutdown()`
- `processFrame()`
- `enterIdleMode()`

### GameWindowTracker

`GameWindowTracker` detects and tracks the Forza Horizon 6 window.

Responsibilities:

- Find the target game process and window.
- Determine whether the game is running and visible.
- Determine the display containing the active game window.
- Refresh cached window and display information.
- Detect changes caused by alt-tab, minimize, resolution changes, or monitor changes.

Key methods:

- `findGameWindow()`
- `isGameRunning()`
- `isGameVisible()`
- `getActiveDisplay()`
- `refresh()`

### IFrameCapture and DesktopFrameCapture

`IFrameCapture` defines the frame acquisition interface. `DesktopFrameCapture` is the Windows desktop capture implementation.

Responsibilities:

- Start capture for the active game display.
- Capture frames continuously while the game is visible.
- Respect the configured frame rate limit.
- Stop capture when idle or shutting down.
- Handle display changes without crashing.
- Fail gracefully when capture is unavailable.

Key methods:

- `start(display)`
- `captureFrame()`
- `stop()`
- `isAvailable()`
- `handleDisplayChanged(display)`

### Frame and FrameRegion

`Frame` represents a captured display frame. `FrameRegion` represents a bounded view into part of that frame.

Responsibilities:

- Store frame dimensions, timestamp, and pixel data.
- Provide region cropping for HUD analysis.
- Provide color sampling helpers for detection and classification.

Key methods:

- `Frame::crop(region)`
- `FrameRegion::averageColor()`

### GearDetector

`GearDetector` identifies the gear HUD region and recognizes the current gear value.

Responsibilities:

- Use the configured or calibrated gear region.
- Extract the gear area from each frame.
- Recognize numeric and supported non-numeric gear states.
- Produce a confidence-scored `GearDetectionResult`.
- Return `Unknown` when the result is ambiguous.

Key methods:

- `setRegion(region)`
- `detectGear(frame)`
- `extractGearRegion(frame)`
- `recognizeGear(region)`

### GearColorClassifier

`GearColorClassifier` determines whether the gear indicator is white, red, or unknown.

Responsibilities:

- Classify color using thresholds rather than exact pixel matching.
- Account for anti-aliasing, brightness, HDR tone mapping, UI scaling, and capture artifacts where feasible.
- Avoid false positives from unrelated red UI elements by operating only on the gear region.
- Return `Unknown` when confidence is too low.

Key methods:

- `classify(region)`
- `updateThresholds(thresholds)`
- `calculateConfidence(region, target)`

### CalibrationService

`CalibrationService` manages the screen region used for gear detection.

Responsibilities:

- Store the current gear indicator region.
- Support manual region selection.
- Optionally attempt automatic gear region detection.
- Restore default region values.

Key methods:

- `autoLocateGearRegion(frame)`
- `setManualRegion(region)`
- `getGearRegion()`
- `resetToDefaults()`

### ShiftAlertController

`ShiftAlertController` owns the alert state machine.

Responsibilities:

- Track the previous confirmed gear color.
- Trigger only on a confirmed white-to-red transition.
- Suppress repeated triggers while the gear remains red.
- Reset trigger readiness when the gear returns to white or another non-red state.
- Keep the alert active for the configured duration.
- Ignore unknown color states for transition triggering.

Key methods:

- `update(result, now)`
- `reset()`
- `isAlertActive(now)`
- `isWhiteToRedTransition(current)`

### OverlayWindow

`OverlayWindow` manages the transparent topmost overlay.

Responsibilities:

- Create a transparent overlay on the active game display.
- Keep the overlay above the game while an alert is active.
- Center the arrow on the active display.
- Ensure the overlay is click-through and does not take focus.
- Hide the arrow when no alert is active.
- Destroy overlay resources on shutdown.

Key methods:

- `create(display)`
- `showArrow()`
- `hideArrow()`
- `centerOnDisplay(display)`
- `ensureClickThrough()`
- `destroy()`

### ArrowRenderer

`ArrowRenderer` draws the shift arrow into the overlay surface.

Responsibilities:

- Render a red arrow.
- Respect the configured size, with the default set to 50x100 pixels.
- Render without flicker.
- Keep rendering independent from window management.

Key methods:

- `render(target)`
- `setColor(color)`
- `setSize(size)`

### AppConfig and ConfigStore

`AppConfig` contains user-adjustable settings. `ConfigStore` handles persistence.

Configuration values:

- Gear indicator region.
- White and red color thresholds.
- Arrow visibility duration.
- Arrow size.
- Target display.
- Frame capture rate limit.
- Diagnostics enabled flag.

Responsibilities:

- Load configuration on startup.
- Save configuration changes locally.
- Restore defaults.
- Keep configuration data local by default.

Key methods:

- `load()`
- `save(config)`
- `restoreDefaults()`

### DiagnosticsService

`DiagnosticsService` exposes status and optional logs.

Responsibilities:

- Report whether the game is detected.
- Report frame acquisition status.
- Report gear and color detection results.
- Report overlay status.
- Provide optional troubleshooting logs.
- Keep logging minimal or disabled by default.

Key methods:

- `setEnabled(enabled)`
- `reportGameDetected(detected)`
- `reportFrameStatus(status)`
- `reportDetection(result)`
- `reportOverlayStatus(status)`
- `log(message)`

## Proposed File Structure

The following file structure keeps the design modular without adding unnecessary layers.

```text
FH6-Head-Up-Display/
  CMakeLists.txt
  docs/
    requirements.md
    uml.puml
    design.md
  src/
    main.cpp
    app/
      HudApplication.h
      HudApplication.cpp
    capture/
      IFrameCapture.h
      DesktopFrameCapture.h
      DesktopFrameCapture.cpp
      Frame.h
      Frame.cpp
      FrameRegion.h
    detection/
      GearDetector.h
      GearDetector.cpp
      GearColorClassifier.h
      GearColorClassifier.cpp
      GearDetectionResult.h
      CalibrationService.h
      CalibrationService.cpp
    alert/
      ShiftAlertController.h
      ShiftAlertController.cpp
      ShiftAlertState.h
    overlay/
      OverlayWindow.h
      OverlayWindow.cpp
      ArrowRenderer.h
      ArrowRenderer.cpp
    config/
      AppConfig.h
      ConfigStore.h
      ConfigStore.cpp
    diagnostics/
      DiagnosticsService.h
      DiagnosticsService.cpp
    platform/
      GameWindowTracker.h
      GameWindowTracker.cpp
      WindowsTypes.h
    shared/
      Color.h
      ColorThreshold.h
      DisplayInfo.h
      Geometry.h
      GearTypes.h
      Time.h
  tests/
    detection/
      GearColorClassifierTests.cpp
      GearDetectorTests.cpp
    alert/
      ShiftAlertControllerTests.cpp
    config/
      ConfigStoreTests.cpp
    support/
      FakeFrameCapture.h
      TestFrames.h
```

The exact build system and dependency choices can be adjusted later. The structure assumes CMake because it is common for modern C++ Windows projects and works well with unit test targets.

## Data Flow

### Startup Flow

1. The user starts the application.
2. `HudApplication::initialize()` loads `AppConfig` through `ConfigStore`.
3. `HudApplication` configures `GearDetector`, `GearColorClassifier`, `ShiftAlertController`, `OverlayWindow`, and `DiagnosticsService`.
4. `GameWindowTracker` searches for the Forza Horizon 6 process and window.
5. If the game is found and visible, `DesktopFrameCapture` starts on the active game display.
6. If the game is not found, the application enters idle mode and periodically retries.

### Per-Frame Flow

1. `HudApplication::processFrame()` asks `GameWindowTracker` to refresh game status.
2. If the game is not running or not visible, the application stops or pauses capture and enters idle mode.
3. `DesktopFrameCapture::captureFrame()` returns the latest available frame.
4. `GearDetector::detectGear(frame)` crops the configured gear region and recognizes the gear value.
5. `GearColorClassifier::classify(region)` classifies the color state.
6. `GearDetectionResult` combines gear value, color state, confidence, and region.
7. `ShiftAlertController::update(result, now)` evaluates the transition state.
8. If the alert is active, `OverlayWindow::showArrow()` displays the centered arrow.
9. If the alert is inactive, `OverlayWindow::hideArrow()` hides the arrow.
10. `DiagnosticsService` receives status updates for game, frame, detection, and overlay state.

### White-to-Red Alert Flow

The alert logic treats color classification as a state machine:

- `White` followed by `Red` triggers the arrow.
- `Red` followed by `Red` keeps the current alert state but does not retrigger.
- `Red` followed by `White` resets the controller so another future `White` to `Red` transition can trigger.
- `Unknown` does not trigger the arrow.
- `Unknown` should not be used as proof that a valid white-to-red transition occurred.

### Idle and Recovery Flow

The application enters idle mode when:

- The game process is not running.
- The game window is minimized.
- The active game display cannot be captured.
- Frame acquisition temporarily fails.

While idle, the application keeps diagnostics available and periodically retries detection and capture initialization. It should not crash or leave the overlay visible during idle mode.

## Overlay Design

The overlay is a transparent, topmost, non-focusable, click-through window.

Required behavior:

- It must not steal focus from Forza Horizon 6.
- It must not receive keyboard or mouse input.
- It must not interfere with controller, wheel, pedal, or other game input devices.
- It should not appear as a separate active gameplay surface in the taskbar.
- It should be positioned on the display that contains the game window.
- It should render the arrow centered in display coordinates.

The arrow renderer should be independent from the overlay window. This allows rendering to be tested or replaced without changing window management.

## Configuration Design

Configuration is local-only by default.

Recommended defaults:

- Arrow size: 50x100 pixels.
- Arrow color: red.
- Gear color state threshold: conservative enough to avoid false positives.
- Diagnostics: disabled or minimal.
- Capture rate limit: configurable to balance latency and performance.

The exact storage format can be chosen later. A simple JSON or INI file is sufficient if it supports readable settings, defaults, and safe recovery from malformed files.

## Diagnostics Design

Diagnostics should help answer four questions quickly:

- Is Forza Horizon 6 detected?
- Are frames being captured?
- Is the gear value detected confidently?
- Is the gear color classified confidently?

The diagnostics layer should avoid storing gameplay frames by default. If future diagnostic screenshots are added, they should require explicit user action or consent.

## Error Handling

The application should fail softly:

- If the game is missing, enter idle mode.
- If capture fails, retry and report frame status.
- If detection is ambiguous, return `Unknown` and do not trigger the arrow.
- If overlay creation fails, continue diagnostics and avoid crashing.
- If configuration loading fails, restore defaults and report the issue.

No error path should disrupt gameplay or change game focus.

## Test Strategy

### Unit Tests

Unit tests should cover logic that can run without the game or Windows capture APIs.

Priority tests:

- `ShiftAlertController` triggers on `White` to `Red`.
- `ShiftAlertController` does not repeatedly trigger while state remains `Red`.
- `ShiftAlertController` resets after `Red` to `White`.
- `ShiftAlertController` ignores `Unknown` states.
- `GearColorClassifier` classifies representative white, red, and ambiguous regions.
- `GearColorClassifier` rejects low-confidence classifications.
- `ConfigStore` loads defaults when no config exists.
- `ConfigStore` restores defaults after malformed config.

### Detection Tests

Detection tests should use synthetic and captured sample frame regions.

Coverage should include:

- White gear indicator samples.
- Red gear indicator samples.
- Anti-aliased edges.
- Brightness and contrast variation.
- HDR or tone-mapped color variation where available.
- Obscured or ambiguous gear regions.
- Non-numeric gear states such as neutral or reverse if present in the HUD.

### Integration Tests

Integration tests should validate component interaction without requiring live gameplay where possible.

Suggested tests:

- `HudApplication` with a fake frame capture source and fake overlay.
- Game missing starts idle mode.
- Game visible starts frame processing.
- Frame acquisition failure enters recoverable state.
- White-to-red sample frame sequence causes one overlay show event.
- Repeated red frames do not cause repeated show events.
- Return to white allows a later red transition to trigger again.

### Manual Tests

Manual testing is required for platform-specific behavior and live gameplay conditions.

Manual scenarios:

- Run the app before launching Forza Horizon 6.
- Run the app after Forza Horizon 6 is already running.
- Test windowed and borderless fullscreen modes.
- Alt-tab away from and back to the game.
- Minimize and restore the game.
- Change resolution or HUD scale.
- Use keyboard, mouse, controller, wheel, and pedals while the overlay is active.
- Verify the game remains focused when the arrow appears.
- Verify the arrow is centered and exactly 50x100 pixels on the active display.
- Verify multi-monitor behavior.
- Verify gameplay performance remains acceptable.

### Performance Tests

Performance tests should measure:

- Time from red gear detection to overlay visibility.
- Average and peak frame processing time.
- CPU usage.
- GPU usage where capture and overlay rendering use GPU resources.
- Memory usage over an extended gameplay session.
- Behavior under configured frame rate limits.

### Regression Tests

Regression tests should be added for every fixed detection bug or alert state bug. Sample frames that expose detection failures should be retained as test fixtures when privacy and copyright considerations allow.

## Design Risks

- The exact Forza Horizon 6 HUD layout and gear indicator visuals may differ from assumptions.
- HDR and display scaling may make color classification less stable.
- Desktop capture behavior may vary between exclusive fullscreen, borderless fullscreen, and windowed modes.
- Some overlay techniques may behave differently depending on graphics settings, monitor configuration, or Windows version.
- Gear recognition may require calibration or image recognition improvements once real game frames are available.

## Future Extensions

- Calibration UI for selecting the gear indicator region.
- Optional preview mode for testing detection outside live gameplay.
- Additional shift cue styles.
- Per-car or per-HUD-profile configuration.
- Exportable diagnostic bundle with explicit user consent.
