# Cruce Design

## Architecture Overview

Cruce uses a server-authoritative architecture. The server owns the real match state, validates every bid and card play, resolves tricks, calculates scores, and decides when a match ends. Clients are presentation and input layers: they render the state they are allowed to see and send player actions to the server.

The core game rules should be written in shared C++ so the same logic can be used by the server, unit tests, bots, and the web client through WebAssembly. The Windows desktop client is a Python/Tk application that talks to the local server API and does not contain authoritative rules. The mobile client should remain C++-heavy where practical. The web client should use TypeScript/HTML/CSS for browser UI and call into the compiled C++ rules module where useful for local previews, disabled-card hints, and validation messages. The server remains the final authority even when clients run local validation.

Persistent data is stored through a repository layer. PostgreSQL is the production target, with SQLite allowed for local development and automated tests.

## Main Components

## Server

`GameServer` manages rooms, player connections, reconnection, incoming actions, and outgoing state updates. It accepts WebSocket messages from all client platforms and routes them to the active `Match`.

`Room` represents a lobby before a match starts. It stores the room code, selected player count, target score of 6, 11, or 21, and seated players. When the room is full, it can create a `Match`.

`PrivacyFilter` creates different state snapshots for each player. Public state includes seating, scores, bids, trump, trick history, current turn, and visible table cards. Private state adds only the receiving player's hand. This is the boundary that prevents clients from seeing hidden cards.

`GameRepository` persists active matches, completed match records, round results, participants, teams, scores, timestamps, and winners.

## Core Game Domain

`Match` is the top-level game object. It owns players, optional teams, the scoreboard, dealer position, target score, match status, and the active round. It starts rounds, applies bids and card plays, finishes rounds, rotates the dealer, and checks whether the match is complete.

`Round` owns the state for one deal: deck, hands, bidding state, trump suit, bid winner, tricks, announcements, and round result. A round ends when all cards for that mode have been played.

`RulesEngine` contains pure rule logic. It creates the 24-card deck, validates bids, validates legal card plays, resolves trick winners, detects announcements, computes round scores, applies failed-bid penalties, and checks winning conditions. This should be deterministic and easy to test.

`Deck` owns the shuffled cards and supports dealing and drawing. `Card` stores suit, rank, point value, and comparison behavior for trick resolution. `Trick` stores cards played in order and determines the winner based on led suit and trump.

`Player` stores player identity, display name, platform, seat, connection state, and hand. In 4-player matches, `Team` groups opposite seats and receives shared score. In 2- and 3-player matches, each player is scored individually.

`Bid`, `Announcement`, `ScoreBoard`, `MatchRecord`, and `RoundResult` are small model objects for explicit state and persistence.

## Client Applications

All clients share the same interaction model:

- Connect or reconnect to a room.
- Render public table state.
- Render only the local player's hand.
- Send bids, trump choices, and card plays.
- Display rejected actions with clear reasons.
- Show round and match scores.

`MobileClient`, `WindowsDesktopClient`, and `WebClient` inherit the same conceptual `ClientApp` behavior from the UML. They do not own authoritative rules or hidden information. Client-side rule checks are helpful for disabling illegal cards, but the server must always revalidate.

The implemented Windows desktop UI lives in `src/app/windows_client.py`. It uses the same HTTP endpoints as the browser client for login, lobby polling, invitations, waiting-room chat, target selection, bidding, trump choice, and card play.

## Proposed File Structure

This structure keeps C++ rules central and separates platform-specific UI from shared game behavior.

```text
src/
  core/
    card/
    match/
    rules/
    scoring/
  server/
    networking/
    rooms/
    persistence/
  clients/
    mobile/
    web/
  app/
    windows_client.py
  shared/
    protocol/
    serialization/
tests/
  cpp/
    core/
    server/
  python/
    integration/
    bots/
docs/
  requirements.md
  uml.puml
  design.md
tools/
  scripts/
  migrations/
```

The exact filenames can be chosen during implementation, but the dependency direction should stay clear: clients and server may depend on `core`, while `core` must not depend on UI, networking, or database code.

## Data Flow

## Room Creation and Match Start

1. A client asks `GameServer` to create a room with player count 2, 3, or 4 and target score 6, 11, or 21.
2. `Room` stores the setup and waits for enough players.
3. Players join from any supported platform using the room code.
4. When the room is full, `Room.startMatch()` creates a `Match`.
5. `Match.startRound()` creates a `Round`, deals cards, and enters bidding.
6. `PrivacyFilter` sends each player a personalized initial state.

## Player Action Flow

1. The active client sends a bid, trump choice, or card play over WebSocket.
2. `GameServer` checks the connection and routes the action to the active `Match`.
3. `Match` asks `RulesEngine` to validate the action.
4. If invalid, the server returns a rejection reason and leaves match state unchanged.
5. If valid, the match state changes, the repository saves the event or state, and `PrivacyFilter` broadcasts updated snapshots.

## Trick and Round Flow

1. The trick leader plays first.
2. Players act clockwise.
3. `RulesEngine.validateCardPlay()` enforces follow-suit, trump, and winning-card rules when applicable.
4. `Trick.winner()` determines who leads next.
5. In 2-player mode, the trick winner draws first while the draw pile has cards.
6. At round end, `RulesEngine.scoreRound()` calculates card points, announcements, bid success or failure, and match-point changes.
7. `ScoreBoard` applies the result and checks the selected target score.

## Privacy and Security Flow

The server stores full state, but clients never receive full state. Before any message leaves the server, `PrivacyFilter` produces a player-specific response. Tests should confirm that another player's hand is absent from normal updates, reconnect payloads, persistence APIs, logs, and completed match history.

## Persistence Design

The first persistent records should be:

- Player identity and display name.
- Room setup: player count, target score, creation time.
- Match record: participants, teams, target score, final score, winner, timestamps.
- Round result: dealer, bid winner, bid value, trump, score deltas, announcements.
- Event audit: accepted actions and rejected actions with reason.

Active matches may be periodically snapshotted for reconnect support. Completed matches should be immutable except for administrative correction tools added later.

## Test Strategy

## Unit Tests

C++ unit tests should cover the shared core first:

- Deck creation has exactly 24 unique cards.
- Dealing works for 2-, 3-, and 4-player modes.
- Bidding accepts only increasing bids from 1 to 4 and handles all-pass redeals.
- Trump selection belongs to the bid winner.
- Trick resolution handles led suit, trump, and required winning plays.
- 2-player draw-pile rules differ correctly before and after the draw pile is empty.
- Announcements score 20 or 40 points and can only happen once per suit per round.
- Round scoring applies successful bids, failed bids, and non-bidder scoring.
- Match completion uses target scores 6, 11, and 21 and continues tied games.

## Server Tests

Server tests should verify room creation, joining, mixed-platform seating, reconnection, rejected illegal actions, state persistence, and per-player state filtering. These tests should run without real clients by driving the WebSocket protocol directly.

## Integration Tests

Python integration tests should run scripted matches with bot players. They should cover complete 2-, 3-, and 4-player matches, including a 4-player team match and at least one reconnect scenario.

## Client Tests

Client tests should verify that each platform can render the same public state, show only the local hand, submit legal actions, display validation errors, and show final match results. The web client should also verify that the WebAssembly rules module matches the server's C++ behavior.

## Load and Reliability Tests

Automated bots should simulate at least 100 concurrent active matches to validate the first production target. Tests should track latency for turn updates, reconnect success, server memory usage, and persistence errors.
