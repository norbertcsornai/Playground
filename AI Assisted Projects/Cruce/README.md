# Cruce

Cruce is a server-authoritative multiplayer card game implementation based on the requirements in `docs/requirements.md`. The current codebase provides a C++ rules/server vertical slice with browser, Python Windows desktop, and Kotlin Android clients, login/register, online player invitations, privacy-filtered snapshots, local test apps, and automated tests.

## Current Scope

- C++20 core rules for cards, deck, bidding, trump, trick validation, announcements, scoring, and target-score win checks.
- C++ server facade for rooms, joining, reconnecting, action validation, persistence records, and per-player hidden-card filtering.
- File-backed local user database in `data/users.db`, spectator-safe match history in `data/match_history.db`, and JSONL move logs for AI training in `data/game_events.db`.
- C++ client facades for mobile, Windows desktop, web, and bot players.
- Local C++ HTTP test server with built-in browser login, lobby, waiting room, and match pages.
- Python/Tk Windows desktop client and Kotlin Android client.
- Cropped card image assets under `assets/cards/`, generated from the `tromf.ro` tutorial sprite.
- CTest-based C++ tests and a Python smoke test.

The production WebSocket transport, PostgreSQL adapter, and WebAssembly packaging are possible next implementation layers.

## Project Structure

```text
src/
  app/        Demo executable
  clients/    Platform client facades
  core/       Shared C++ game domain and rules
  server/     Room, server, repository, and privacy filtering
assets/       Card images copied/cropped from tromf.ro
data/         Local user, match history, and move-log databases
tests/
  cpp/        C++ unit and server tests
  python/     Python integration smoke test
docs/         Requirements, UML, and design notes
tools/        Utility scripts
```

## Build

Use a C++20 compiler and CMake 3.20 or newer. On Windows, run these from a Visual Studio Developer PowerShell or Developer Command Prompt.

```sh
cmake -S . -B build
cmake --build build
```

With NMake from a Visual Studio developer shell:

```sh
cmake -S . -B build -G "NMake Makefiles"
cmake --build build
```

## Run Tests

```sh
ctest --test-dir build -C Debug --output-on-failure
```

For single-config generators, omit `-C Debug`.

This runs:

- `cruce_core_tests`
- `cruce_server_tests`
- `cruce_python_smoke`, when Python is available to CMake
- `cruce_local_server_smoke`, on Windows when Python is available

## Run the Demo

```sh
build/cruce_demo
```

On multi-config generators such as Visual Studio, the executable may be under a configuration directory:

```sh
build/Debug/cruce_demo.exe
```

The demo creates a 4-player room with mixed client platforms, starts a match, and prints a privacy-filtered view for one player.

## Run the Local Test Apps

Start the C++ local server:

```sh
build/Debug/cruce_server_app.exe 8080
```

Open the browser client:

```text
http://127.0.0.1:8080
```

Start the Python Windows desktop client:

```sh
python src/app/windows_client.py
```

Start the Android client:

1. Open the `android/` folder in Android Studio.
2. Connect your Android phone with USB debugging enabled, or choose an emulator.
3. Press `Run`, or build a debug APK with Gradle:

```sh
cd android
gradle :app:assembleDebug
```

The debug APK is written to `android/app/build/outputs/apk/debug/app-debug.apk`.
When using a physical phone, keep the phone on the same router as the PC and enter the PC's LAN address in the Android app, for example `http://192.168.2.17:8080`. Do not use `127.0.0.1` on the phone because that points back to the phone itself. If the phone cannot connect, allow TCP port `8080` through Windows Firewall for the Cruce server.

Browser, Windows, and Android clients require login or registration. To create an account, enter a username and password and press `Register`; then use the same credentials to log in from any client. After login, clients show online players, profiles, wins, and recent match history. A player can invite another online player or built-in AI bot (`ai_bot_1` through `ai_bot_4`) to a 2-, 3-, or 4-player game. AI bots accept invitations, choose target scores when selected, and play legal turns automatically. Larger games wait in a chat room until enough players join. Each player sees only their own cards and can bid, choose trump, and play highlighted legal cards through server-validated actions. In the Python Windows client, double-click a card image or select it and press `Play Selected Card`; in Android, tap a highlighted card.

## Generate AI Training Data

The server executable can generate valid bot-versus-bot games without opening clients:

```sh
build/Debug/cruce_server_app.exe --self-play 500
```

By default this appends 500 completed 6-point games with mixed 2-, 3-, and 4-player tables to `data/game_events.db` and `data/match_history.db`. Use options such as `--players 4`, `--target 11`, `--seed 123`, `--events path`, or `--history path` to control the batch.

## Train Baseline AI Models

Export compact examples from the move log:

```sh
python tools/export_training_data.py --include-real
```

Train the baseline bid, trump, and card-play models:

```sh
python src/ai/train.py --data data/ai --models data/ai/models/v1 --epochs 3
```

The models are dependency-free sparse linear classifiers saved under `data/ai/models/v1/`. They imitate the current self-play and real-player decisions. Card prediction uses `legalCardIds` as a mask so an AI policy does not choose illegal cards.

To test a trained model against a private match state JSON:

```sh
python src/ai/policy.py --models data/ai/models/v1 --state state.json
```

To let the trained AI generate more games through the normal server HTTP API:

```sh
python tools/ai_self_play.py --models data/ai/models/v1 --games 15 --players 2
```

After generating model-guided games, rerun export and training into a new version directory such as `data/ai/models/v2/`, then compare `metrics.json` files.

```sh
python tools/compare_ai_models.py data/ai/models/v1 data/ai/models/v2
```

## Card Assets

The 24 playable card images are stored in `assets/cards/`. To regenerate them from the `tromf.ro` sprite:

```powershell
powershell -ExecutionPolicy Bypass -File tools/download_tromf_cards.ps1
```

## Development Notes

The server is authoritative: clients never receive another player's hand, and every bid or card play is revalidated by the server. The current local auth database uses deterministic password hashes suitable for development only; production should replace this with a stronger password-hashing scheme and TLS-only credential submission.
