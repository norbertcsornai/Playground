# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A Windows-only C++20 desktop app that runs alongside Forza Horizon 6. It watches the game's gear indicator via desktop-frame capture, classifies its color (white/red/unknown), and shows a transparent red arrow overlay on white-to-red transitions as a shift cue. It never touches game memory, files, or input — it's a passive capture + overlay tool (see "Safety Model" in README.md).

The project is still early: gear *value* OCR is not implemented (`GearDetector::recognizeGear` returns `Unknown`), only gear *color* classification is functional. `docs/requirements.md` and `docs/design.md` are the source-of-truth specs this implementation was built from — consult them before changing behavior in `alert/`, `detection/`, or `overlay/`, since they define the intended state machine and acceptance criteria.

## Build

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

Use `--config Release` for a release build. (An existing `build-cmake433/` directory is a local build output dir — anything matching `build/` or `build-*/` is gitignored.)

Run the **Release** build when testing real shift-cue latency. This is a soft-real-time capture loop, and MSVC Debug runs the per-pixel conversion loop ~14x slower (measured: 91ms vs 6.5ms per 1080p frame), which dominates any other latency work.

## Run tests

```powershell
cd build
ctest -C Debug --output-on-failure
cd ..
```

Tests are a single executable (`fh6-hud-tests`) built from all files under `tests/`, using a small custom header-only framework (`tests/TestFramework.h` — `FH6_TEST(name)` registers a test via static init, `FH6_REQUIRE(cond)` asserts). There is no built-in test filter — `ctest` and running `fh6-hud-tests.exe` directly both run the entire suite. To exercise a single test in isolation, temporarily comment out the other `FH6_TEST` blocks or run the binary and read the `[PASS]`/`[FAIL]` per-test output lines it prints.

Adding a new test file requires registering it explicitly in `CMakeLists.txt` under the `fh6-hud-tests` executable's source list — files aren't picked up by glob.

## Run

```powershell
.\build\Debug\fh6-hud.exe
```

Start FH6 first (or the app idles and retries). Config lives at `%APPDATA%\FH6HeadUpDisplay\config.ini`; defaults apply if absent.

## Architecture

`HudApplication` (`src/app/`) is the orchestrator and owns every other component. `run()` is a synchronous loop calling `processFrame()` every ~5ms; there is no threading inside the pipeline. Per-frame flow:

1. `GameWindowTracker` (`src/platform/`) checks the FH6 window is running/visible and returns the active `DisplayInfo` → if not, `enterIdleMode()` stops capture, resets alert state, hides the overlay, and the loop returns early.
2. `IFrameCapture` / `DesktopFrameCapture` (`src/capture/`) grabs a `Frame` from the active display (DXGI desktop duplication, falling back to GDI), rate-limited by `captureRateLimitFps`. `HudApplication` calls `setRegionOfInterest()` with the HUD rect so only those pixels are read off the GPU and converted — converting a whole screen per frame was previously the dominant cost. The resulting `Frame` therefore holds only a sub-rectangle: rects passed to `crop()`/`clippedBounds()` are always in display-local coordinates (0,0 = display top-left), with `origin()` locating the frame's pixels in that space and `sourceWidth()/sourceHeight()` reporting full display size for region math.
3. `GearDetector` (`src/detection/`) crops the configured `gearRegion` out of the frame (via `CalibrationService`) and would recognize the gear value (currently stubbed to `Unknown`).
4. `GearColorClassifier` (`src/detection/`) classifies the cropped region as white/red/unknown. This is the functional core: it doesn't do simple average-color matching — it inspects a widget's ring vs. inner-glyph vs. background pixel ratios (`WidgetColorStats`/`calculateWidgetColorStats`) against configurable `ColorThresholds`, specifically to reject other red HUD elements (see `2c2bbb0 Reduce HUD red false positives`). Confidence must clear `confidenceThreshold_` and `minimumGlyphPixels_` or the result is `Unknown`.
5. `ShiftAlertController` (`src/alert/`) is a small state machine: only a confirmed White→Red edge triggers an alert; repeated Red does not retrigger; Red→White (or Unknown) resets trigger-readiness; alert stays active for `arrowDuration`. `Unknown` never counts as proof of a transition.
6. `OverlayWindow` + `ArrowRenderer` (`src/overlay/`) draw a topmost, transparent, click-through window centered on the active display. **Invariant: the window is shown once in `create()` and never hidden or repositioned again in the hot path.** Showing/hiding a topmost *layered* window, or calling `SetWindowPos(HWND_TOPMOST)`, forces the window manager to recompute z-order and the compositor to re-evaluate the fullscreen game beneath it — doing that per shift cue visibly stutters the game. The arrow is toggled purely by what gets painted: the window is colour-keyed on black, so filling it black renders it fully transparent. Do not reintroduce `ShowWindow`/`SetWindowPos` to toggle the cue; `tests/overlay/OverlayWindowTests.cpp` guards this.
7. `DiagnosticsService` (`src/diagnostics/`) records per-stage status (game detected, frame status, detection result, overlay status); disabled by default.

`AppConfig` (`src/config/AppConfig.h`) is the plain settings struct; `ConfigStore` loads/saves it to the INI file, falling back to defaults on missing/malformed config. `HudApplication` has a second constructor taking an injected `IFrameCapture` + `ConfigStore`, used to build fakes for testing without touching real Windows capture APIs.

Shared value types (`Color`, `ColorThreshold(s)`, `Rect`/`Size`/`Geometry`, `GearValue`/`GearColorState`, `DisplayInfo`, `Status` enums, `Time`/`Clock`) live in `src/shared/` and are included across module boundaries — check there first before adding a new small value type.

## Code style notes

Existing source files carry a trailing `// ...` comment on nearly every line restating what that line does mechanically (e.g. `#include <memory>  // Imports the memory standard library declarations used in this file.`). This is pervasive throughout `src/` and `tests/` as currently written. Match the surrounding file's existing convention when editing it; don't introduce this style in new code you write from scratch, and don't take it as license to over-comment non-obvious logic explanations — the project's own default (per general guidance) is to comment only the non-obvious "why".
