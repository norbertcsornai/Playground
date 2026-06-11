# FH6 Head-Up Display Requirements

## Overview

This document defines requirements for a C++ Windows desktop application that runs alongside Forza Horizon 6 on a PC. The application reads each rendered game frame, identifies the current gear value and gear indicator color, and displays a red shift arrow when the gear indicator changes from white to red.

The application must not disrupt the game, steal focus, block inputs, or interfere with normal gameplay.

## Scope

### In Scope

- Capture or inspect visible game frames while Forza Horizon 6 is running on the same PC.
- Detect the current gear value from the game's HUD.
- Detect the displayed color state of the gear value.
- Determine when the gear indicator changes from white to red.
- Display a red arrow overlay at the center of the screen when the shift condition is met.
- Keep the overlay non-interactive so mouse, keyboard, controller, and wheel inputs continue to reach the game.
- Run as a standalone C++ Windows application.

### Out of Scope

- Modifying Forza Horizon 6 game files, memory, network traffic, telemetry, or executable behavior.
- Automating game inputs or changing vehicle behavior.
- Supporting platforms other than Windows PC.
- Providing a full HUD replacement.
- Persisting gameplay recordings or screenshots unless explicitly enabled for diagnostics.

## Assumptions

- Forza Horizon 6 is running in a mode that allows desktop capture or frame inspection.
- The gear indicator is visible in the game HUD.
- The gear indicator has visually distinguishable white and red states.
- The target display resolution and HUD scale may vary between users.
- The application should avoid techniques that trigger anti-cheat, platform protection, or game integrity systems.

## Functional Requirements

### Frame Acquisition

- The application shall detect when Forza Horizon 6 is running.
- The application shall acquire frame data from the display containing the game.
- The application shall process frames continuously while the game is active.
- The application shall support borderless fullscreen and windowed display modes where technically feasible.
- The application shall handle game minimize, alt-tab, resolution changes, and display changes without crashing.
- The application shall stop frame processing or enter an idle state when the game is not running or is not visible.

### Gear Detection

- The application shall identify the screen region containing the current gear indicator.
- The application shall read the current gear value from each processed frame.
- The application shall support numeric gears and non-numeric gear states shown by the game, such as neutral or reverse, if they are present in the HUD.
- The application shall tolerate minor visual changes caused by anti-aliasing, motion blur, brightness, HDR tone mapping, UI scaling, and compression artifacts from capture.
- The application shall expose a calibration or configuration mechanism for the gear indicator region if automatic detection is unreliable.

### Gear Color Detection

- The application shall determine whether the gear indicator is currently in a white state, red state, or unknown state.
- The application shall compare color using a threshold-based or otherwise robust method rather than exact pixel matching.
- The application shall ignore frames where the gear indicator color cannot be determined confidently.
- The application shall avoid treating unrelated red UI elements as the gear indicator color.

### Shift Alert Logic

- The application shall detect a transition from a confirmed white gear indicator state to a confirmed red gear indicator state.
- The application shall display the shift arrow when the white-to-red transition is detected.
- The application shall avoid repeatedly triggering the arrow every frame while the gear indicator remains red.
- The application shall reset its trigger state once the gear indicator returns to white or another non-red state.
- The application shall provide configurable timing for how long the arrow remains visible after a trigger.

### Overlay Display

- The application shall display a red arrow overlay centered on the screen.
- The arrow shall be 50 pixels wide and 100 pixels tall.
- The arrow shall be visually red and clearly visible over gameplay.
- The overlay shall have a transparent background.
- The overlay shall not appear in the Windows taskbar as a separate active gameplay surface.
- The overlay shall remain above the game window while the alert is active.
- The overlay shall hide when no alert is active.
- The overlay shall support the active game display when multiple monitors are connected.

### Input and Focus Behavior

- The overlay shall not capture keyboard input.
- The overlay shall not capture mouse input.
- The overlay shall not capture controller, wheel, pedal, or other game input devices.
- The application shall not change the focused window away from Forza Horizon 6 during gameplay.
- The application shall not inject inputs into the game.

### Configuration

- The application shall provide configurable settings for gear indicator region, color thresholds, arrow visibility duration, target display, and frame processing rate.
- The application shall save configuration locally.
- The application shall load saved configuration on startup.
- The application shall provide a way to restore default settings.

### Diagnostics

- The application shall expose diagnostic status for whether the game is detected, frames are being processed, the gear value is detected, and the gear color state is detected.
- The application shall provide optional logging for troubleshooting detection failures.
- Diagnostic logging shall be disabled or minimal by default to avoid performance impact.

## Non-Functional Requirements

### Performance

- The application should process frames with low latency so the arrow appears quickly after the gear turns red.
- The application should keep CPU and GPU usage low enough to avoid noticeable gameplay performance degradation.
- The application should allow frame processing rate limits to balance responsiveness and performance.
- The overlay display should update without visible flicker.

### Reliability

- The application shall not crash when Forza Horizon 6 starts, exits, minimizes, changes resolution, or changes display mode.
- The application shall recover automatically from temporary frame acquisition failures.
- The application shall fail gracefully when frame capture is unavailable.

### Compatibility

- The application shall run on supported Windows PC environments for modern C++ desktop applications.
- The application should support common monitor configurations, including single monitor, multiple monitors, and high-DPI displays.
- The application should account for SDR and HDR display differences where feasible.

### Safety and Game Integrity

- The application shall not modify game files.
- The application shall not read or write game process memory.
- The application shall not hook game input APIs.
- The application shall not automate gameplay.
- The application should rely on desktop capture and overlay techniques that are compatible with normal Windows application behavior.

### Usability

- The application should be simple to start before or during gameplay.
- The application should make its current status understandable without requiring the user to inspect logs.
- The application should provide calibration controls for users whose HUD layout, resolution, or color output differs from defaults.

### Maintainability

- The C++ codebase should separate frame acquisition, gear recognition, color classification, alert logic, overlay rendering, configuration, and diagnostics into distinct components.
- The codebase should include automated tests for detection logic and alert state transitions where practical.
- Requirements and implementation decisions should be documented as the project evolves.

### Privacy

- The application shall not upload gameplay frames, screenshots, logs, or configuration data to external services by default.
- Any future external diagnostics or telemetry shall require explicit user consent.

## User Stories

- As a player, I want the app to run while Forza Horizon 6 is running so I can receive shift timing feedback during gameplay.
- As a player, I want the app to detect when the gear indicator turns red so I know when to shift.
- As a player, I want a clear red arrow at the center of the screen so I can see the shift cue without looking away from the road.
- As a player, I want the overlay to be click-through and non-interactive so it never interferes with steering, shifting, braking, menus, or camera controls.
- As a player, I want the app to keep working after alt-tabbing or changing display modes so I do not need to restart it during a session.
- As a player with a custom HUD scale or display setup, I want calibration settings so detection can be adjusted to my screen.
- As a developer, I want detection, overlay, and alert logic separated into clear components so the project can be tested and maintained.
- As a developer, I want diagnostic information so I can understand whether failures come from frame capture, gear recognition, color classification, or overlay display.

## Acceptance Criteria

### Game Detection

- Given Forza Horizon 6 is not running, when the application starts, then it reports that the game is not detected and remains idle.
- Given Forza Horizon 6 starts after the application is already running, when the game window becomes available, then the application detects it and begins frame processing.
- Given Forza Horizon 6 exits, when the game process or window disappears, then the application stops frame processing without crashing.

### Frame Processing

- Given the game is visible, when frames are available, then the application processes frames continuously.
- Given the game is minimized or unavailable for capture, when frame acquisition fails, then the application reports the failure and retries without crashing.
- Given the game resolution changes, when frame acquisition resumes, then the application continues processing using the updated dimensions.

### Gear and Color Detection

- Given the gear indicator is visible and white, when a frame is processed, then the application classifies the gear color as white with sufficient confidence.
- Given the gear indicator is visible and red, when a frame is processed, then the application classifies the gear color as red with sufficient confidence.
- Given the gear indicator is obscured or ambiguous, when a frame is processed, then the application classifies the gear color as unknown and does not trigger the arrow.
- Given a calibrated gear indicator region, when the game HUD scale changes within supported limits, then the application can still detect the gear value and color after recalibration.

### Shift Arrow Trigger

- Given the previous confirmed gear color is white, when the current confirmed gear color becomes red, then the application displays the red arrow.
- Given the gear indicator remains red for multiple frames, when frames continue to be processed, then the application does not repeatedly retrigger the arrow every frame.
- Given the gear indicator returns to white or another non-red state, when a later white-to-red transition occurs, then the application can trigger the arrow again.
- Given the gear color is unknown, when the next frame is red, then the application does not trigger unless a prior confirmed white state was observed.

### Overlay Behavior

- Given a shift alert is active, when the arrow is displayed, then it appears centered on the active game display.
- Given a shift alert is active, when the arrow is displayed, then it is 50 pixels wide and 100 pixels tall.
- Given no shift alert is active, when the overlay is visible, then the arrow is hidden.
- Given the player moves the mouse or presses keys while the overlay is active, then the game continues receiving input normally.
- Given the player uses a controller, wheel, or pedals while the overlay is active, then the game continues receiving input normally.
- Given the game has focus, when the overlay appears, then Forza Horizon 6 remains the focused application.

### Configuration and Diagnostics

- Given the user changes detection or overlay settings, when the application closes and reopens, then the settings are restored.
- Given the user restores defaults, when settings are reloaded, then default values are applied.
- Given diagnostics are enabled, when detection fails, then the application reports whether the issue is game detection, frame acquisition, gear recognition, color classification, or overlay rendering.

### Performance and Stability

- Given the application runs during gameplay, when the arrow triggers, then the cue appears with low enough latency to be useful for shifting.
- Given the application runs for an extended gameplay session, then it remains stable and does not leak resources enough to degrade gameplay.
- Given frame processing is enabled, then gameplay remains responsive and input latency is not noticeably affected by the application.

## Open Questions

- What exact HUD location and visual style will Forza Horizon 6 use for the gear indicator?
- Which capture API should be used to balance compatibility, latency, and game integrity?
- Should the arrow point upward, downward, or use a specific shift-light design?
- Should the application support an optional preview or calibration mode outside gameplay?
- What latency target should be considered acceptable for competitive or high-speed driving?
