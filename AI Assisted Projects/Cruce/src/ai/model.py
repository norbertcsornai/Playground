"""Small sparse linear classifiers for Cruce AI.

This deliberately avoids third-party dependencies. It is not meant to be the
final strongest model; it is a durable baseline that can train anywhere Python
runs.
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Iterable


class SparsePerceptron:
    def __init__(self, labels: Iterable[str]):
        self.labels = list(labels)
        self.label_to_index = {label: index for index, label in enumerate(self.labels)}
        self.weights: dict[str, list[float]] = {}

    def score(self, features: Iterable[str], allowed: Iterable[str] | None = None) -> tuple[str, list[float]]:
        if allowed is None:
            indices = list(range(len(self.labels)))
        else:
            indices = [
                self.label_to_index[label]
                for label in allowed
                if label in self.label_to_index
            ]
            if not indices:
                indices = list(range(len(self.labels)))

        scores = [0.0] * len(self.labels)
        for feature in features:
            vector = self.weights.get(feature)
            if vector is None:
                continue
            for index in indices:
                scores[index] += vector[index]

        best = max(indices, key=lambda index: (scores[index], -index))
        return self.labels[best], scores

    def update(self, features: Iterable[str], expected: str, predicted: str, rate: float = 1.0) -> None:
        if expected == predicted:
            return
        expected_index = self.label_to_index[expected]
        predicted_index = self.label_to_index[predicted]
        for feature in features:
            vector = self.weights.setdefault(feature, [0.0] * len(self.labels))
            vector[expected_index] += rate
            vector[predicted_index] -= rate

    def prune(self, minimum_abs_weight: float = 0.0001) -> None:
        remove = []
        for feature, vector in self.weights.items():
            if all(abs(value) < minimum_abs_weight for value in vector):
                remove.append(feature)
        for feature in remove:
            del self.weights[feature]

    def save(self, path: Path, metadata: dict | None = None) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "kind": "sparse_perceptron",
            "labels": self.labels,
            "weights": self.weights,
            "metadata": metadata or {},
        }
        path.write_text(json.dumps(payload, separators=(",", ":")), encoding="utf-8")

    @classmethod
    def load(cls, path: Path) -> "SparsePerceptron":
        payload = json.loads(path.read_text(encoding="utf-8"))
        model = cls(payload["labels"])
        model.weights = {
            str(feature): [float(value) for value in vector]
            for feature, vector in payload.get("weights", {}).items()
        }
        return model
