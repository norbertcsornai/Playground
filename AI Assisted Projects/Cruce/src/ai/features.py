"""Feature extraction for Cruce AI models.

The feature format is intentionally plain strings so models can be trained and
loaded without third-party Python dependencies.
"""

from __future__ import annotations

from collections import Counter
from hashlib import sha1
from typing import Iterable


SUITS = ("Hearts", "Diamonds", "Clubs", "Spades")
RANKS = ("Ace", "Ten", "King", "Queen", "Jack", "Nine")
CARD_IDS = tuple(suit * 10 + rank for suit in range(4) for rank in range(6))
CARD_LABELS = tuple(str(card_id) for card_id in CARD_IDS)
BID_LABELS = tuple(str(value) for value in range(5))
TRUMP_LABELS = SUITS

RANK_POINTS = {
    "Ace": 11,
    "Ten": 10,
    "King": 4,
    "Queen": 3,
    "Jack": 2,
    "Nine": 0,
}
RANK_STRENGTH = {
    "Ace": 6,
    "Ten": 5,
    "King": 4,
    "Queen": 3,
    "Jack": 2,
    "Nine": 1,
}


def bucket(value: int, size: int, maximum: int) -> int:
    """Compress a numeric value into a small category for the linear model."""
    if value <= 0:
        return 0
    return min(maximum, value // size)


def card_rank(card: dict) -> str:
    """Read a card rank from either the structured field or the display label."""
    return str(card.get("rank") or str(card.get("label", "")).split(" of ")[0])


def card_suit(card: dict) -> str:
    """Read a card suit, using Unknown when the input is incomplete."""
    return str(card.get("suit", "Unknown"))


def card_id(card: dict) -> int:
    """Return the stable numeric card id used by the C++ server and datasets."""
    return int(card.get("id", -1))


def player_owner_map(state: dict) -> dict[str, str]:
    """Map each player to the owner that scores for them.

    In 2- and 3-player games the owner is usually the player id. In 4-player
    team games two players share an owner such as team-0 or team-1.
    """
    return {
        str(player.get("id", "")): str(player.get("owner") or player.get("id", ""))
        for player in state.get("players", [])
    }


def actor_owner(state: dict, actor: str) -> str:
    """Return the scoring owner for the player whose action we are modeling."""
    return player_owner_map(state).get(actor, actor)


def card_beats(challenger: dict, current_winner: dict, led_suit: str, trump_suit: str) -> bool:
    """Apply the same trick comparison idea used by the C++ rule engine."""
    challenger_suit = card_suit(challenger)
    winner_suit = card_suit(current_winner)
    if challenger_suit == winner_suit:
        return RANK_STRENGTH.get(card_rank(challenger), 0) > RANK_STRENGTH.get(
            card_rank(current_winner), 0
        )
    if challenger_suit == trump_suit and winner_suit != trump_suit:
        return True
    if winner_suit == trump_suit and challenger_suit != trump_suit:
        return False
    return challenger_suit == led_suit and winner_suit != led_suit


def current_trick_winner(state: dict) -> dict | None:
    """Find the currently winning played card in the partial trick."""
    trick = list(state.get("currentTrick", []))
    trump = state.get("trump")
    if not trick or not trump:
        return None
    led_suit = card_suit(trick[0].get("card", {}))
    winner = trick[0]
    for played in trick[1:]:
        if card_beats(played.get("card", {}), winner.get("card", {}), led_suit, trump):
            winner = played
    return winner


def has_pair_card(hand: list[dict], card: dict) -> bool:
    """Check whether playing this King/Queen could announce a marriage."""
    rank = card_rank(card)
    if rank not in {"King", "Queen"}:
        return False
    pair_rank = "Queen" if rank == "King" else "King"
    suit = card_suit(card)
    return any(card_suit(other) == suit and card_rank(other) == pair_rank for other in hand)


def game_key(event: dict) -> str:
    """Create a stable key so all examples from one game share one split."""
    state = event.get("stateBefore") or event.get("stateAfter") or {}
    players = sorted(str(player.get("id", "")) for player in state.get("players", []))
    raw = "|".join([str(event.get("matchId", "")), *players])
    return sha1(raw.encode("utf-8")).hexdigest()


def split_name(key: str, eval_percent: int = 20) -> str:
    """Deterministically choose train or eval without splitting one game twice."""
    value = int(sha1(key.encode("utf-8")).hexdigest()[:8], 16) % 100
    return "eval" if value < eval_percent else "train"


def state_features(state: dict, actor: str) -> list[str]:
    """Turn one private match snapshot into plain string features.

    The model does not understand cards directly. It learns from strings such as
    hand_suit_count:Hearts:3, legal_wins_current:10, or own_score:2.
    """
    features: list[str] = ["bias"]
    players = state.get("players", [])
    player_count = len(players)

    # Public match context tells the model which phase and scoring target it is in.
    features.append(f"players:{player_count}")
    features.append(f"target:{state.get('targetScore', 0)}")
    features.append(f"status:{state.get('status', '')}")
    features.append(f"dealer:{state.get('dealerSeat', -1)}")
    features.append(f"turn_is_actor:{state.get('currentTurn') == actor}")

    # Seat and owner features help the model learn turn order and team ownership.
    owner = actor_owner(state, actor)
    actor_seat = -1
    actor_cards = 0
    for player in players:
        seat = int(player.get("seat", -1))
        cards = int(player.get("cardsInHand", 0))
        features.append(f"seat_cards:{seat}:{cards}")
        if player.get("id") == actor:
            actor_seat = seat
            actor_cards = cards
            features.append(f"actor_connected:{bool(player.get('connected', True))}")
            features.append(f"actor_owner:{player.get('owner', actor)}")
            features.append(f"actor_team:{bool(player.get('team'))}")
    features.append(f"actor_seat:{actor_seat}")
    features.append(f"actor_cards:{actor_cards}")

    trump = state.get("trump") or "None"
    features.append(f"trump:{trump}")

    # Hand features summarize the private cards only this player is allowed to see.
    hand = list(state.get("ownHand", []))
    hand_points = 0
    hand_strength = 0
    suit_counts: Counter[str] = Counter()
    suit_points: Counter[str] = Counter()
    ranks_by_suit: dict[str, set[str]] = {suit: set() for suit in SUITS}
    for card in hand:
        cid = card_id(card)
        rank = card_rank(card)
        suit = card_suit(card)
        points = int(card.get("points", RANK_POINTS.get(rank, 0)))
        strength = RANK_STRENGTH.get(rank, 0)
        hand_points += points
        hand_strength += strength
        suit_counts[suit] += 1
        suit_points[suit] += points
        ranks_by_suit.setdefault(suit, set()).add(rank)
        features.append(f"hand:{cid}")
        features.append(f"hand_rank:{rank}")
        features.append(f"hand_suit:{suit}")

    features.append(f"hand_count:{len(hand)}")
    features.append(f"hand_points:{bucket(hand_points, 8, 12)}")
    features.append(f"hand_strength:{bucket(hand_strength, 6, 10)}")
    for suit in SUITS:
        features.append(f"hand_suit_count:{suit}:{suit_counts[suit]}")
        features.append(f"hand_suit_points:{suit}:{bucket(suit_points[suit], 8, 8)}")
        if {"King", "Queen"}.issubset(ranks_by_suit.get(suit, set())):
            features.append(f"marriage:{suit}")

    # The best trump candidate helps trump and bid models learn suit preference.
    best_trump_suit = max(
        SUITS,
        key=lambda suit: (
            suit_counts[suit] * 12
            + suit_points[suit] * 2
            + sum(RANK_STRENGTH.get(rank, 0) for rank in ranks_by_suit.get(suit, set())) * 4
            + (34 if {"King", "Queen"}.issubset(ranks_by_suit.get(suit, set())) else 0)
        ),
    )
    features.append(f"best_trump_candidate:{best_trump_suit}")

    # Legal card ids are server-filtered, so the card model learns from valid choices.
    legal_ids = [int(value) for value in state.get("legalCardIds", [])]
    features.append(f"legal_count:{len(legal_ids)}")
    for cid in legal_ids:
        features.append(f"legal:{cid}")

    trick = list(state.get("currentTrick", []))
    features.append(f"trick_len:{len(trick)}")
    trick_points = 0

    # Trick features describe the cards already on the table.
    if trick:
        led = card_suit(trick[0].get("card", {}))
        features.append(f"led_suit:{led}")
    for index, played in enumerate(trick):
        card = played.get("card", {})
        trick_points += int(card.get("points", RANK_POINTS.get(card_rank(card), 0)))
        features.append(f"trick_pos:{index}:{card_id(card)}")
        features.append(f"trick_suit:{card_suit(card)}")
        features.append(f"trick_rank:{card_rank(card)}")
        features.append(f"trick_seat:{index}:{played.get('seat', -1)}")
    features.append(f"trick_points:{bucket(trick_points, 8, 15)}")
    features.append(f"last_to_play:{bool(trick) and len(trick) + 1 >= player_count}")

    owner_map = player_owner_map(state)
    winner = current_trick_winner(state)
    winner_owner = ""

    # Knowing whether our side is winning is critical in 4-player team games.
    if winner:
        winner_player = str(winner.get("player", ""))
        winner_owner = owner_map.get(winner_player, winner_player)
        features.append(f"current_winner_owner:{'own' if winner_owner == owner else 'other'}")

    # Per-legal-card features teach the model which valid cards can win, feed
    # points to our side, or trigger announcements.
    hand_by_id = {card_id(card): card for card in hand}
    legal_can_win = False
    for cid in legal_ids:
        card = hand_by_id.get(cid)
        if not card:
            continue
        rank = card_rank(card)
        suit = card_suit(card)
        points = int(card.get("points", RANK_POINTS.get(rank, 0)))
        features.append(f"legal_rank:{rank}")
        features.append(f"legal_suit:{suit}")
        features.append(f"legal_points:{cid}:{bucket(points, 3, 4)}")
        if winner and state.get("trump") and card_beats(
            card, winner.get("card", {}), card_suit(trick[0].get("card", {})), state["trump"]
        ):
            features.append(f"legal_wins_current:{cid}")
            legal_can_win = True
        if winner_owner == owner and points >= 4:
            features.append(f"legal_feeds_own_side:{cid}")
        if has_pair_card(hand, card):
            announcement_kind = "trump" if suit == trump else "plain"
            features.append(f"legal_announcement:{cid}:{announcement_kind}")
    features.append(f"legal_can_win:{legal_can_win}")

    bids = list(state.get("bids", []))
    highest = 0

    # Bid history lets the model know whether it must raise or can pass.
    for bid in bids:
        passed = bool(bid.get("passed"))
        value = int(bid.get("value", 0))
        if not passed:
            highest = max(highest, value)
        prefix = "pass" if passed else "bid"
        features.append(f"bid_seen:{prefix}:{value}")
        if bid.get("player") == actor:
            features.append(f"actor_bid_seen:{prefix}:{value}")
    features.append(f"bid_count:{len(bids)}")
    features.append(f"highest_bid:{highest}")

    scores = state.get("scores") or {}
    round_points = state.get("roundPoints") or {}

    # Score features let the model act differently when close to winning or losing.
    own_score = int(scores.get(owner, 0))
    own_round = int(round_points.get(owner, 0))
    best_other = max((int(score) for key, score in scores.items() if key != owner), default=0)
    features.append(f"own_score:{bucket(own_score, 2, 12)}")
    features.append(f"score_diff:{bucket(own_score - best_other + 20, 4, 10)}")
    features.append(f"own_round_points:{bucket(own_round, 16, 12)}")

    if state.get("lastTrickWinner"):
        features.append(f"last_trick_winner_is_actor:{state.get('lastTrickWinner') == actor}")

    return features


def legal_card_mask(state: dict) -> list[str]:
    """Return allowed card labels so inference cannot select an illegal card."""
    return [str(int(card_id)) for card_id in state.get("legalCardIds", [])]


def class_labels(task: str) -> tuple[str, ...]:
    """Return the possible output labels for one prediction task."""
    if task == "bid":
        return BID_LABELS
    if task == "trump":
        return TRUMP_LABELS
    if task == "card":
        return CARD_LABELS
    raise ValueError(f"Unknown task: {task}")


def unique_features(features: Iterable[str]) -> list[str]:
    """Remove duplicate feature strings to keep training examples compact."""
    return sorted(set(features))
