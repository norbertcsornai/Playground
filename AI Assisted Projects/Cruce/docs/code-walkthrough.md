# Code Walkthrough

This guide explains the main code paths without forcing every source line to
carry a comment. Use it as a map while reading the code.

## Game Core

`src/core/types.*` defines shared values such as `Suit`, `Rank`, `Card`,
`Bid`, and `MatchStatus`. These types are used everywhere so server, clients,
tests, and AI agree on the same card ids and rules language.

`src/core/rules_engine.*` contains rule checks that do not depend on UI:
bidding validation, legal card play validation, trick winner resolution, card
points, announcement points, and round scoring.

`src/core/match.*` owns the state of a match. It deals cards, tracks whose turn
it is, accepts bids, accepts trump selection, validates card plays, scores
tricks, starts new rounds, and marks the match complete when an owner reaches
the selected target.

## Server

`src/server/game_server.*` manages rooms and active matches. Clients never edit
match state directly; they call server methods such as `submit_bid`,
`choose_trump`, and `submit_card_play`.

`src/server/privacy_filter.*` creates a private `GameSnapshot` for one player.
This is why each client sees only its own hand while still seeing public table
state.

`src/app/server_main.cpp` contains the local HTTP app, browser UI, lobby,
invitations, rematches, move logging, and AI bot behavior.

## AI Flow

`CruceHeuristicBot` in `src/app/server_main.cpp` is the live hand-written bot.
It chooses bids from hand strength, chooses trump from suit strength, and picks
cards from trick context. In 4-player games it checks team ownership so it does
not waste cards overtaking a partner.

`SelfPlaySimulator` in the same file runs bot-vs-bot games without opening a
client. It logs every decision to `data/game_events.db` and completed match
summaries to `data/match_history.db`.

`tools/export_training_data.py` reads the event log and writes JSONL datasets
under `data/ai/`. It creates separate datasets for bidding, trump selection,
and card play.

`src/ai/features.py` converts a private game snapshot into plain string
features. Examples include hand strength, legal cards, current trick winner,
announcement opportunities, score pressure, and team ownership.

`src/ai/train.py` trains three sparse perceptron models: one for bids, one for
trump, and one for card choice. The card model receives a legal-card mask so it
cannot choose cards the server would reject.

`src/ai/policy.py` loads trained models and predicts the next action from one
private state JSON. `tools/ai_self_play.py` uses that policy through the normal
HTTP API to generate model-guided games.

## Client Flow

The browser UI is embedded in `src/app/server_main.cpp`. The Windows client is
`src/app/windows_client.py`. The Android client is in `android/app/src/main/`.
All clients talk to the same server endpoints and receive privacy-filtered
state.

## Useful Commands

```sh
cmake --build build
ctest --test-dir build -C Debug --output-on-failure
build/Debug/cruce_server_app.exe --self-play 500 --players 2 --target 6
python tools/export_training_data.py --include-real
python src/ai/train.py --data data/ai --models data/ai/models/v3 --epochs 5
```
