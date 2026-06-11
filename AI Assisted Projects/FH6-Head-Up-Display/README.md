# FH6 Head-Up Display

A modern C++ Windows desktop application prototype for displaying a shift cue while Forza Horizon 6 is running.

The app watches the game display, samples the configured gear indicator region, classifies the gear color as white, red, or unknown, and shows a 300x600 pixel red arrow at the center of the active display when the gear indicator transitions from white to red.

## Project Status

This is an initial implementation based on:

- `docs/requirements.md`
- `docs/uml.puml`
- `docs/design.md`

Implemented:

- Modular C++ architecture under `src/`
- Windows game-window tracking
- Windows GDI desktop frame capture
- Transparent click-through overlay window
- Gear color classification
- White-to-red shift alert state machine
- Local configuration store
- Diagnostics service
- Unit tests under `tests/`

Not yet complete:

- Real Forza Horizon 6 gear OCR/template recognition
- Calibration UI
- DirectX Desktop Duplication or Windows Graphics Capture backend
- Polished diagnostics UI

The current implementation uses a configured gear HUD region and color classification. Gear value recognition currently returns `Unknown` until real FH6 frame samples are available.

## Requirements

- Windows PC
- CMake 3.18 or newer
- A C++20 compiler, such as Visual Studio 2022 MSVC

## Build

From the repository root:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

For a Debug build:

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

## Run Tests

```powershell
cd build
ctest -C Debug --output-on-failure
cd ..
```

If you built Release, use:

```powershell
cd build
ctest -C Release --output-on-failure
cd ..
```

## Run

Build the app, start Forza Horizon 6, then run:

```powershell
.\build\Release\fh6-hud.exe
```

For Debug:

```powershell
.\build\Debug\fh6-hud.exe
```

Press `Ctrl+C` in the terminal to stop the app.

## Configuration

On Windows, the app loads configuration from:

```text
%APPDATA%\FH6HeadUpDisplay\config.ini
```

If the file does not exist, defaults are used.

Important defaults:

- Arrow size: `75x150`
- Arrow position: centered horizontally, `200 px` above screen center
- Arrow opacity: `100%`
- Arrow duration: `1500 ms`
- Capture rate limit: `30 FPS`
- Gear region: dynamic lower-right default, scaled from the captured frame height
- Diagnostics: disabled

The default gear region targets the lower-right HUD cluster, then the detector focuses on the circled gear value inside that cluster so speed digits and tachometer redline marks are ignored. For real gameplay use, it may still need calibration to the actual Forza Horizon 6 gear indicator location for the player's resolution and HUD scale.

## Safety Model

The app is designed to avoid disrupting the game:

- It does not modify game files.
- It does not read or write game process memory.
- It does not inject inputs.
- It does not hook game input APIs.
- The overlay is intended to be topmost, transparent, non-focusable, and click-through.

## Documentation

- Requirements: `docs/requirements.md`
- UML: `docs/uml.puml`
- Design: `docs/design.md`
