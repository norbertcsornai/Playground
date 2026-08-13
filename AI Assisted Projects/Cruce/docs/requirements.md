# Cruce Requirements

## Purpose and Scope

Cruce is a multiplayer trick-taking card game playable by 2, 3, or 4 people. Any player may join from the mobile client, Windows desktop client, or web client in the same match. The server is the authority for cards, turns, scoring, and privacy.

## Technology Direction

- Use C++20 or newer for the core game rules, server game engine, validation, scoring, and shared client logic.
- Use CMake for C++ builds and dependency management.
- Use a C++ real-time server with WebSocket or equivalent bidirectional transport.
- Use Python/Tk for the Windows desktop client, while keeping authoritative game logic in C++.
- Use a C++-heavy mobile client unless platform research proves another approach is better.
- Use WebAssembly for sharing the C++ rules engine with the web client, with TypeScript/HTML/CSS for browser UI.
- Use Python for developer tooling, integration tests, load-test bots, migrations, and automation where it is more practical than C++.
- Use PostgreSQL for persistent user, match, round, score, and audit data. SQLite may be used for local development.

## Game Rules

### Players and Teams

- A match supports exactly 2, 3, or 4 players.
- In 2-player and 3-player games, each player competes individually.
- In 4-player games, there are 2 teams of 2 players. Players seated opposite each other are teammates.
- The server must allow mixed-platform play in every mode.

### Deck

- Use a 24-card deck: 4 suits with 6 ranks each.
- Supported display suits: Hearts, Diamonds, Clubs, Spades. Regional names may be added later.
- Rank strength from highest to lowest: Ace, Ten, King, Queen, Jack, Nine.
- Card point values:

| Rank | Points |
| --- | ---: |
| Ace | 11 |
| Ten | 10 |
| King | 4 |
| Queen | 3 |
| Jack | 2 |
| Nine | 0 |

- Total card points in a round are 120, excluding announcements.

### Match and Round Flow

- A match is played over multiple rounds.
- Before the match starts, the room selects a target score of 6, 11, or 21 match points.
- After each round, the server records the round result and updates match scores.
- The first individual player or 4-player team to reach or exceed the selected target score wins the match.
- If multiple players or teams are tied at or above the target score, play continues until one player or team is ahead after round scoring.
- The dealer rotates clockwise each round.

### Dealing

- 4 players: each player receives 6 cards.
- 3 players: each player receives 8 cards.
- 2 players: each player receives 8 cards, and 8 cards remain face down as the draw pile.

### Bidding and Trump

- Starting clockwise after the dealer, each player may pass or bid a number from 1 to 4.
- Each bid point represents 33 card points.
- Each new bid must be higher than the current highest bid.
- If all players pass, the round is redealt by the next dealer.
- The highest bidder chooses the trump suit and leads the first trick.
- The first card of the first trick must be trump.

### Trick Play

- The trick leader plays first.
- Players act clockwise.
- In 3-player and 4-player games, each player must follow the led suit if possible.
- If a player cannot follow suit, the player must play trump if possible.
- If a player cannot follow suit or play trump, the player may play any card.
- A player must play a card that is currently winning the trick if they can legally do so.
- A trick is won by the highest trump card played. If no trump is played, it is won by the highest card of the led suit.
- The trick winner leads the next trick.
- In 2-player games, while the draw pile has cards, players may play any card. After each trick, the trick winner draws first, then the other player draws. Once the draw pile is empty, normal follow-suit and trump rules apply.

### Announcements

- A player holding King and Queen of the same suit may score an announcement.
- A non-trump announcement is worth 20 card points.
- A trump announcement is worth 40 card points.
- The announcement is made when the player first plays one card from that pair.
- Each suit may be announced at most once per round.

### Round Scoring

- At round end, each player or team totals card points from won tricks plus announcement points.
- Match points earned by a player or team are `floor(total_round_points / 33)`.
- If the bidder's player/team earns at least the bid value, they receive their earned match points.
- If the bidder's player/team fails the bid, they lose match points equal to the bid.
- Non-bidding players or teams always receive their earned match points.

### Hidden Information

- A player may only view their own hand.
- Teammates may not view each other's cards.
- Clients may display public information only: seating, turn, bids, trump, cards already played to the current trick, completed trick history, announcements, and scores.
- The server must never send hidden cards to unauthorized clients.

## Functional Requirements

- FR-1: Users can create or join private rooms for 2, 3, or 4 players and select a target score of 6, 11, or 21.
- FR-2: Users can join the same room from mobile, Windows desktop, or web.
- FR-3: The server starts a match only when the selected player count is met.
- FR-4: The server shuffles, deals, validates bids, validates card plays, resolves tricks, and scores rounds.
- FR-5: Clients must show only the local player's hand and public match state.
- FR-6: The game must persist match results, round results, player/team scores, and final winners.
- FR-7: Players can reconnect to an active match and resume from the current server state.
- FR-8: Illegal bids or card plays are rejected with a clear client-visible reason.
- FR-9: The UI must support bidding, trump selection, card play, score review, and match completion.
- FR-10: The system must support automated bot players for testing and future practice mode.

## Non-Functional Requirements

- NFR-1: Server decisions must be deterministic and testable from replayed match events.
- NFR-2: Typical turn actions should be reflected to all clients within 250 ms on a normal broadband connection.
- NFR-3: All network traffic must use TLS outside local development.
- NFR-4: The system must prevent clients from inferring hidden cards through API payloads or logs.
- NFR-5: The server must support at least 100 concurrent active matches in the first production target.
- NFR-6: Shared C++ game rules must have unit tests for dealing, bidding, trick resolution, announcements, and scoring.
- NFR-7: Clients must remain usable on small mobile screens and standard desktop resolutions.
- NFR-8: Errors, disconnects, and timeouts must be logged with match and player identifiers.

## User Stories

- As a player, I want to create a 2-, 3-, or 4-player room so I can choose the game mode.
- As a player, I want to join from any supported platform so I can play with friends using different devices.
- As a player, I want to see only my own cards so the game remains fair.
- As a bidder, I want the app to enforce valid bids so the round starts correctly.
- As an active player, I want illegal cards disabled or rejected so I understand what I can play.
- As a 4-player participant, I want my teammate and opponents clearly identified.
- As a returning player, I want reconnect support so a temporary network issue does not end my match.
- As a player, I want match history to show scores and winners so completed games are recorded.

## Acceptance Criteria

- AC-1: A 2-player, 3-player, and 4-player match can each be completed from room creation through final score.
- AC-2: A 4-player match always assigns opposite seats as teammates and scores by team.
- AC-3: A mixed-platform match works with one player on each available client type when enough players are present.
- AC-4: No client receives another player's hand in normal play, reconnect, logs, or match history APIs.
- AC-5: The server rejects an out-of-turn action, invalid bid, or illegal card play without changing match state.
- AC-6: Round scoring follows the bid success and failed-bid rules exactly.
- AC-7: The match ends after round scoring when one player or team has reached or exceeded the selected target score and is not tied for first place.
- AC-8: Completed match records include participants, teams if any, target score, final score, winner, round scores, and timestamps.
- AC-9: Automated tests cover all core rule paths for 2-, 3-, and 4-player modes.

## Out of Scope for the First Version

- Public matchmaking, ranking ladders, tournaments, spectators, gambling, and real-money features.
- Regional rule variants such as double rounds, blind trump selection, alternate deck names, or custom target scores.
- In-game voice chat.
