from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from ai.features import game_key, legal_card_mask, split_name, state_features, unique_features


ACTION_TASK = {
    "self_play_bid": "bid",
    "ai_bid": "bid",
    "bid": "bid",
    "self_play_choose_trump": "trump",
    "ai_choose_trump": "trump",
    "choose_trump": "trump",
    "self_play_play_card": "card",
    "ai_play_card": "card",
    "play_card": "card",
}


def label_for(task: str, request: dict) -> str | None:
    if task == "bid":
        if "value" not in request:
            return None
        return str(int(request["value"]))
    if task == "trump":
        suit = request.get("suit")
        return str(suit) if suit else None
    if task == "card":
        if "cardId" not in request:
            return None
        return str(int(request["cardId"]))
    return None


def valid_example(task: str, label: str, state: dict) -> bool:
    if task == "card":
        mask = set(legal_card_mask(state))
        hand = {str(int(card.get("id", -1))) for card in state.get("ownHand", [])}
        return label in mask and label in hand
    return True


def open_outputs(out_dir: Path):
    out_dir.mkdir(parents=True, exist_ok=True)
    handles = {}
    for task in ("bid", "trump", "card"):
        for split in ("train", "eval"):
            handles[(task, split)] = (out_dir / f"{task}_{split}.jsonl").open(
                "w", encoding="utf-8"
            )
    return handles


def export(events_path: Path, out_dir: Path, eval_percent: int, include_real: bool) -> Counter:
    counts: Counter[str] = Counter()
    handles = open_outputs(out_dir)
    try:
        with events_path.open("r", encoding="utf-8") as source:
            for line_number, line in enumerate(source, start=1):
                if not line.strip():
                    continue
                try:
                    event = json.loads(line)
                except json.JSONDecodeError:
                    counts["parse_error"] += 1
                    continue

                action = event.get("action")
                task = ACTION_TASK.get(action)
                if task is None:
                    counts["ignored"] += 1
                    continue
                if not include_real and not str(action).startswith("self_play_"):
                    counts["ignored_real"] += 1
                    continue
                if not event.get("ok", False):
                    counts["ignored_failed"] += 1
                    continue

                state = event.get("stateBefore") or {}
                request = event.get("request") or {}
                actor = str(event.get("actor", ""))
                label = label_for(task, request)
                if label is None or not state or not valid_example(task, label, state):
                    counts[f"invalid_{task}"] += 1
                    continue

                key = game_key(event)
                split = split_name(key, eval_percent)
                example = {
                    "gameKey": key,
                    "matchId": event.get("matchId", ""),
                    "actor": actor,
                    "source": action,
                    "label": label,
                    "features": unique_features(state_features(state, actor)),
                }
                if task == "card":
                    example["mask"] = legal_card_mask(state)

                handles[(task, split)].write(json.dumps(example, separators=(",", ":")) + "\n")
                counts[f"{task}_{split}"] += 1

                if line_number % 50000 == 0:
                    print(f"processed {line_number} events...", flush=True)
    finally:
        for handle in handles.values():
            handle.close()
    return counts


def main() -> int:
    parser = argparse.ArgumentParser(description="Export Cruce AI training datasets.")
    parser.add_argument("--events", default="data/game_events.db", type=Path)
    parser.add_argument("--out", default="data/ai", type=Path)
    parser.add_argument("--eval-percent", default=20, type=int)
    parser.add_argument(
        "--include-real",
        action="store_true",
        help="Include human/client actions in addition to self-play actions.",
    )
    args = parser.parse_args()

    counts = export(args.events, args.out, args.eval_percent, args.include_real)
    print("Export complete.")
    for key, value in sorted(counts.items()):
        print(f"{key}: {value}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
