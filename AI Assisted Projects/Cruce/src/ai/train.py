from __future__ import annotations

import argparse
import json
import random
from collections import Counter
from pathlib import Path
import sys

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from ai.features import class_labels
from ai.model import SparsePerceptron


def read_examples(path: Path) -> list[dict]:
    """Load JSONL examples produced by tools/export_training_data.py."""
    examples = []
    if not path.exists():
        return examples
    with path.open("r", encoding="utf-8") as source:
        for line in source:
            if line.strip():
                examples.append(json.loads(line))
    return examples


def train_task(task: str, data_dir: Path, model_dir: Path, epochs: int, seed: int) -> dict:
    """Train one classifier: bid, trump, or card play."""
    train_path = data_dir / f"{task}_train.jsonl"
    eval_path = data_dir / f"{task}_eval.jsonl"
    train_examples = read_examples(train_path)
    eval_examples = read_examples(eval_path)

    # The sparse perceptron stores one weight vector for every string feature.
    model = SparsePerceptron(class_labels(task))
    rng = random.Random(seed)
    label_counts = Counter(example["label"] for example in train_examples)

    # Each epoch shuffles examples and updates the model whenever it predicts
    # something different from the logged action.
    for epoch in range(1, epochs + 1):
        rng.shuffle(train_examples)
        mistakes = 0
        for example in train_examples:
            mask = example.get("mask")
            predicted, _scores = model.score(example["features"], mask)
            expected = example["label"]
            if predicted != expected:
                mistakes += 1
                model.update(example["features"], expected, predicted)
        print(
            f"{task}: epoch {epoch}/{epochs}, "
            f"mistakes={mistakes}, examples={len(train_examples)}",
            flush=True,
        )

    # Pruning removes near-zero weights so saved model files stay smaller.
    model.prune()
    train_metrics = evaluate(model, train_examples)
    eval_metrics = evaluate(model, eval_examples)

    # Metadata is saved next to the weights to make model versions auditable.
    metadata = {
        "task": task,
        "epochs": epochs,
        "seed": seed,
        "train_examples": len(train_examples),
        "eval_examples": len(eval_examples),
        "label_counts": dict(label_counts),
        "train": train_metrics,
        "eval": eval_metrics,
    }
    model.save(model_dir / f"{task}_model.json", metadata)
    return metadata


def evaluate(model: SparsePerceptron, examples: list[dict]) -> dict:
    """Measure how often the trained model imitates held-out logged actions."""
    if not examples:
        return {"examples": 0, "accuracy": 0.0}
    correct = 0
    total = 0
    by_label: Counter[str] = Counter()
    correct_by_label: Counter[str] = Counter()
    for example in examples:
        expected = example["label"]
        predicted, _scores = model.score(example["features"], example.get("mask"))
        by_label[expected] += 1
        total += 1
        if predicted == expected:
            correct += 1
            correct_by_label[expected] += 1

    per_label = {
        label: {
            "examples": count,
            "accuracy": correct_by_label[label] / count if count else 0.0,
        }
        for label, count in sorted(by_label.items())
    }
    return {
        "examples": total,
        "correct": correct,
        "accuracy": correct / total,
        "per_label": per_label,
    }


def main() -> int:
    """Command-line entry point for training all Cruce AI task models."""
    parser = argparse.ArgumentParser(description="Train Cruce AI baseline models.")
    parser.add_argument("--data", default="data/ai", type=Path)
    parser.add_argument("--models", default="data/ai/models/v1", type=Path)
    parser.add_argument("--epochs", default=3, type=int)
    parser.add_argument("--seed", default=20260813, type=int)
    args = parser.parse_args()

    args.models.mkdir(parents=True, exist_ok=True)
    summary = {}

    # Train one model per decision type because each task has different labels.
    for offset, task in enumerate(("bid", "trump", "card")):
        summary[task] = train_task(
            task,
            args.data,
            args.models,
            args.epochs,
            args.seed + offset,
        )

    summary_path = args.models / "metrics.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(f"Training complete. Metrics written to {summary_path}")
    for task, metrics in summary.items():
        print(
            f"{task}: train_acc={metrics['train']['accuracy']:.3f}, "
            f"eval_acc={metrics['eval']['accuracy']:.3f}, "
            f"train={metrics['train_examples']}, eval={metrics['eval_examples']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
