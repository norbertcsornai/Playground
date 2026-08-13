import json
import subprocess
import sys
import time
import urllib.parse
import urllib.request
from pathlib import Path


PORT = 18081
HUMAN = "ai_smoke_user"
BOT = "ai_bot_1"


def get_json(path: str) -> dict:
    with urllib.request.urlopen(f"http://127.0.0.1:{PORT}{path}", timeout=5) as response:
        return json.loads(response.read().decode("utf-8"))


def enc(text: str) -> str:
    return urllib.parse.quote(text)


def wait_for_server() -> None:
    deadline = time.time() + 8
    while time.time() < deadline:
        try:
            get_json("/api/lobby?username=not_a_user")
            return
        except Exception:
            time.sleep(0.1)
    raise RuntimeError("local server did not become ready")


def login_or_register(username: str, password: str) -> None:
    get_json(f"/api/register?username={enc(username)}&password={enc(password)}&platform=Web")
    login = get_json(f"/api/login?username={enc(username)}&password={enc(password)}&platform=Web")
    assert login["ok"], login


def play_human_turn(state: dict) -> None:
    status = state["status"]
    if status == "Bidding":
        has_bid = any(not bid["passed"] for bid in state["bids"])
        value = 0 if has_bid else 1
        result = get_json(f"/api/bid?username={HUMAN}&value={value}")
    elif status == "ChoosingTrump":
        suit = state["ownHand"][0]["suit"]
        result = get_json(f"/api/trump?username={HUMAN}&suit={suit}")
    elif status == "Playing":
        legal = set(state["legalCardIds"])
        card_id = next(card["id"] for card in state["ownHand"] if card["id"] in legal)
        result = get_json(f"/api/play?username={HUMAN}&card={card_id}")
    else:
        return
    assert result["ok"], result


def finish_ai_match() -> dict:
    for _ in range(800):
        lobby = get_json(f"/api/lobby?username={HUMAN}")
        assert lobby["ok"], lobby
        if not lobby["inMatch"]:
            return lobby
        match = get_json(f"/api/match?username={HUMAN}")
        assert match["ok"], match
        state = match["state"]
        if state.get("currentTurn") == HUMAN:
            play_human_turn(state)
        else:
            time.sleep(0.01)
    raise AssertionError("AI bot match did not finish")


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: ai_bot_smoke.py <cruce_server_app>", file=sys.stderr)
        return 2

    repo_root = Path(__file__).resolve().parents[2]
    server = subprocess.Popen(
        [sys.argv[1], str(PORT)],
        cwd=repo_root,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    try:
        wait_for_server()
        login_or_register(HUMAN, HUMAN)
        lobby = get_json(f"/api/lobby?username={HUMAN}")
        bots = [player for player in lobby["onlinePlayers"] if player["platform"] == "Bot"]
        assert any(player["username"] == BOT for player in bots), lobby

        invite = get_json(f"/api/invite?from={HUMAN}&to={BOT}&players=2")
        assert invite["ok"], invite

        lobby = get_json(f"/api/lobby?username={HUMAN}")
        if lobby.get("targetSelection"):
            target = lobby["targetSelection"]
            assert target["chooser"] == HUMAN, target
            selected = get_json(f"/api/target?username={HUMAN}&score=6")
            assert selected["ok"], selected

        match = get_json(f"/api/match?username={HUMAN}")
        assert match["ok"], match
        assert any(player["id"] == BOT for player in match["state"]["players"]), match

        completed_lobby = finish_ai_match()
        assert completed_lobby["rematch"] is not None, completed_lobby
        assert BOT in completed_lobby["rematch"]["players"], completed_lobby
        print("[PASS] AI bot invite and play smoke")
        return 0
    finally:
        server.terminate()
        try:
            server.wait(timeout=3)
        except subprocess.TimeoutExpired:
            server.kill()


if __name__ == "__main__":
    raise SystemExit(main())
