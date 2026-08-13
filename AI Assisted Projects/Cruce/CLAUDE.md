# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

All C++ commands run from the repo root. On Windows use a Visual Studio Developer PowerShell (the CMake default generator is Visual Studio, so binaries land in `build/Debug/`).

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build -C Debug --output-on-failure   # drop -C Debug for single-config generators
ctest --test-dir build -C Debug -R cruce_core_tests --output-on-failure   # one test target
```

Registered CTest targets: `cruce_core_tests`, `cruce_server_tests`, `cruce_python_smoke`, `cruce_windows_python_compile`, and (Windows only) `cruce_local_server_smoke`. The Python tests are only registered when CMake finds a Python 3 interpreter.

Run the apps — **the server must be started from the repo root**, because it resolves `data/users.db` and `assets/cards/` as relative paths:

```sh
build/Debug/cruce_server_app.exe 8080   # HTTP server + browser client at http://127.0.0.1:8080
build/Debug/cruce_demo.exe              # in-process 4-player demo, prints one privacy-filtered snapshot
python src/app/windows_client.py        # Python/Tk desktop client
```

Android (from `android/`; requires `local.properties` with `sdk.dir`). **`JAVA_HOME` must be overridden** — the machine default is JDK 11 and Gradle 9.3 refuses to start on anything below 17:

```powershell
$env:JAVA_HOME = "C:\Program Files\Java\jdk-26.0.2"
./gradlew :app:assembleDebug   # APK at android/app/build/outputs/apk/debug/app-debug.apk
./gradlew :app:installDebug
```

JDK 26 runs Gradle but does not satisfy `jvmToolchain(17)` — toolchain matching is exact on language version. The foojay resolver in `settings.gradle.kts` auto-provisions a real JDK 17 on first build. Don't raise the toolchain to match the installed JDK; AGP 8.7.3 and Kotlin 2.0.21 cap the JVM target well below 26.

There is no linter or formatter configured. Seeded dev accounts are `admin1/admin1` and `admin2/admin2`.

## Architecture

Server-authoritative trick-taking card game. Clients never hold rules or hidden state; every action is revalidated server-side.

### Layering (dependency direction is enforced by CMake and must stay one-way)

`src/core/` ← `src/server/` ← `src/clients/` ← `src/app/`. `core` must never depend on networking, UI, or storage.

- **`src/core/`** — pure domain. `RulesEngine` is entirely static/deterministic (deck creation, bid validation, card-play legality, trick resolution, announcement points, round scoring, win check). `Match` owns players, teams, scoreboard, dealer rotation, and the active `Round`; `Round` owns one deal's deck, bids, trump, tricks, and announcements. `Match` is the only mutator — `Round`'s fields are private with `friend class Match`.
- **`src/server/`** — `GameServer` orchestrates rooms → matches and maps `PlayerId → match_id`. `PrivacyFilter::private_state_for()` is the single choke point that converts a `Match` into a `GameSnapshot`; it emits `own_hand` only for the requesting player and reduces everyone else to `cards_in_hand` counts. Any new state exposed to clients must go through it. `UserStore` is a plaintext-line file DB (`data/users.db`) with a deterministic dev-only hash.
- **`src/clients/client_app.cpp`** — thin in-process facade (`MobileClient`/`WindowsDesktopClient`/`WebClient` differ only by `ClientPlatform`). Used by `cruce_demo` and the C++ tests, *not* by the real clients.

### The two things called "server"

`cruce::server::GameServer` knows nothing about auth, lobbies, or HTTP. Everything else lives in the `LobbyState` class inside `src/app/server_main.cpp` (~2000 lines, single translation unit): login/register, online-user tracking, invitations, target-score selection, waiting rooms with chat, rematch offers, per-user notices, and JSON serialization. When adding a feature, decide deliberately which side it belongs on — match rules go in `core`/`server`, everything social goes in `LobbyState`.

`server_main.cpp` also contains the **entire browser client** as a raw string literal in `html_page()`. Editing the web UI means editing HTML/JS inside a C++ file and rebuilding.

### Wire protocol

Hand-rolled Winsock HTTP/1.1: single-threaded blocking accept loop, one request per connection, **GET only**, all parameters in the query string, all responses JSON with `Access-Control-Allow-Origin: *`. There is no WebSocket and no locking — clients poll `/api/lobby` and `/api/match` every 2000 ms.

Endpoints (all take `username=`): `/api/login`, `/api/register`, `/api/lobby`, `/api/match`, `/api/bid`, `/api/trump`, `/api/play`, `/api/invite`, `/api/target`, `/api/respond`, `/api/chat`, `/api/rematch`, `/api/cancel`, plus `/api/lobby.txt` and `/api/match.txt` (plain-text debugging views, no client uses them) and `/assets/cards/<file>`. Any unmatched path returns the HTML page.

Cards cross the wire as the integer from `Card::id()` (`suit_index * 10 + rank_index`); snapshot JSON also carries an `image` path under `/assets/cards/`. All three clients (browser JS, `windows_client.py`, `MainActivity.kt`) reimplement the same polling/rendering loop against this API, so a protocol change usually means touching all three.

### Clients

- `src/app/windows_client.py` — Tk, loads card images from the local filesystem via `REPO_ROOT`, so it only works on the same machine as the checkout. Server URL is the module constant `SERVER_URL`.
- `android/app/src/main/java/com/cruce/android/MainActivity.kt` — single Activity, no AndroidX (`android.useAndroidX=false`), no XML layouts; the whole UI is built programmatically. `DEFAULT_SERVER_URL` is a hardcoded LAN address, overridable in-app and persisted to `SharedPreferences`. Uses `HttpURLConnection` on a single-thread executor and requires cleartext HTTP.

### Tests

C++ tests use the hand-rolled harness in `tests/cpp/test_support.hpp` (`expect`, `expect_eq`, `run_test`) — no GoogleTest. Each test is a free function registered by name in `main()` and failures are thrown `std::runtime_error`; the exit code is the failure count. `tests/python/local_server_smoke.py` boots the real `cruce_server_app` binary and drives a full 2-player match over the HTTP API.

## Rules reference

`docs/requirements.md` is the authoritative spec for game rules (24-card deck; Ace/Ten/King/Queen/Jack/Nine at 11/10/4/3/2/0 points; bids 1–4 worth 33 card points each; 2/3/4-player deals of 8/8/6 cards with a draw pile only in 2-player; announcements 20 non-trump / 40 trump; match points = `floor(round_points / 33)`; failed bid subtracts the bid value; target scores 6/11/21). Confirm rule changes against it before touching `RulesEngine`. `docs/design.md` describes the intended end state — its WebSocket transport, PostgreSQL repository, and WebAssembly web client are **not implemented**; the current `GameRepository` is in-memory only and match records are lost when the process exits.
