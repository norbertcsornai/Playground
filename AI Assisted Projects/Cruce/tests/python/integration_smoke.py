import subprocess
import sys


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: integration_smoke.py <cruce_demo>", file=sys.stderr)
        return 2

    result = subprocess.run(
        [sys.argv[1]],
        check=False,
        capture_output=True,
        text=True,
    )

    if result.returncode != 0:
        print(result.stderr, file=sys.stderr)
        return result.returncode

    output = result.stdout
    required = [
        "Cruce demo server started an in-memory match",
        "Players: 4",
        "Target score: 6",
        "Status: Bidding",
        "Ana can see 6 cards",
    ]
    for text in required:
        if text not in output:
            print(f"missing expected output: {text}", file=sys.stderr)
            print(output, file=sys.stderr)
            return 1

    print("[PASS] cruce_demo smoke")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
