from __future__ import annotations

import json
from pathlib import Path
import sys

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from ai.features import legal_card_mask, state_features
from ai.model import SparsePerceptron


class CruceAIPolicy:
    """Runtime wrapper around the three trained Cruce decision models."""

    def __init__(self, model_dir: Path):
        # Each model specializes in one phase of the turn: bidding, trump, or card play.
        self.bid_model = SparsePerceptron.load(model_dir / "bid_model.json")
        self.trump_model = SparsePerceptron.load(model_dir / "trump_model.json")
        self.card_model = SparsePerceptron.load(model_dir / "card_model.json")

    def choose_action(self, state: dict, actor: str) -> dict:
        """Convert a private server snapshot into the next AI action."""
        features = state_features(state, actor)
        status = state.get("status")
        if status == "Bidding":
            # Bid labels are strings in the model file, but the HTTP API expects an int.
            label, scores = self.bid_model.score(features)
            return {"action": "bid", "value": int(label), "scores": score_map(self.bid_model, scores)}
        if status == "ChoosingTrump":
            # Trump labels already match the server's suit names.
            label, scores = self.trump_model.score(features)
            return {"action": "choose_trump", "suit": label, "scores": score_map(self.trump_model, scores)}
        if status == "Playing":
            # The card mask forces the model to choose only from server-approved cards.
            mask = legal_card_mask(state)
            label, scores = self.card_model.score(features, mask)
            return {"action": "play_card", "cardId": int(label), "scores": score_map(self.card_model, scores)}
        return {"action": "wait"}


def score_map(model: SparsePerceptron, scores: list[float]) -> dict[str, float]:
    """Expose model scores in debug output so choices can be inspected."""
    return {label: round(scores[index], 3) for index, label in enumerate(model.labels)}


def main() -> int:
    """CLI helper for testing a trained policy against one saved state JSON."""
    import argparse
    import sys

    parser = argparse.ArgumentParser(description="Predict a Cruce AI action from a private state JSON file.")
    parser.add_argument("--models", default="data/ai/models/v1", type=Path)
    parser.add_argument("--state", type=Path, help="JSON file with a private match state; stdin is used if omitted.")
    parser.add_argument("--actor", help="Actor/player id; defaults to state's currentTurn.")
    args = parser.parse_args()

    raw = args.state.read_text(encoding="utf-8") if args.state else sys.stdin.read()
    state = json.loads(raw)
    actor = args.actor or state.get("currentTurn") or ""
    policy = CruceAIPolicy(args.models)
    print(json.dumps(policy.choose_action(state, actor), indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
