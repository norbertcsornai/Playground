from __future__ import annotations

import argparse
import json
from pathlib import Path


def load_metrics(path: Path) -> dict:
    return json.loads((path / "metrics.json").read_text(encoding="utf-8"))


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare Cruce AI model metrics.")
    parser.add_argument("models", nargs="+", type=Path)
    args = parser.parse_args()

    loaded = [(path, load_metrics(path)) for path in args.models]
    print("model,task,train_examples,eval_examples,train_accuracy,eval_accuracy")
    for path, metrics in loaded:
        for task in ("bid", "trump", "card"):
            task_metrics = metrics.get(task, {})
            print(
                f"{path},{task},"
                f"{task_metrics.get('train_examples', 0)},"
                f"{task_metrics.get('eval_examples', 0)},"
                f"{task_metrics.get('train', {}).get('accuracy', 0.0):.4f},"
                f"{task_metrics.get('eval', {}).get('accuracy', 0.0):.4f}"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
