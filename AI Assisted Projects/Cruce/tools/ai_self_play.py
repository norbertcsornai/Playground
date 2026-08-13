from __future__ import annotations

import argparse
import json
import subprocess
import sys
import time
import urllib.parse
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from ai.policy import CruceAIPolicy


def enc(value: str) -> str:
    """URL-encode values before sending them as query parameters."""
    return urllib.parse.quote(str(value))


class ServerClient:
    """Small HTTP client for the local Cruce server."""

    def __init__(self, base_url: str):
        self.base_url = base_url.rstrip("/")

    def get(self, path: str, **params: object) -> dict:
        """Call one GET endpoint and decode its JSON response."""
        query = urllib.parse.urlencode({key: str(value) for key, value in params.items()})
        url = f"{self.base_url}{path}"
        if query:
            url = f"{url}?{query}"
        with urllib.request.urlopen(url, timeout=5) as response:
            return json.loads(response.read().decode("utf-8"))


def wait_for_server(client: ServerClient) -> None:
    """Poll until the spawned server is ready to answer API requests."""
    deadline = time.time() + 8
    while time.time() < deadline:
        try:
            client.get("/api/lobby", username="not_a_user")
            return
        except Exception:
            time.sleep(0.1)
    raise RuntimeError("Cruce server did not become ready.")


def login_or_register(client: ServerClient, username: str, password: str) -> None:
    """Create a bot account if needed, then log it in as an online Bot user."""
    client.get("/api/register", username=username, password=password, platform="Bot")
    login = client.get("/api/login", username=username, password=password, platform="Bot")
    if not login.get("ok"):
        raise RuntimeError(f"Unable to log in {username}: {login}")


def find_invitation(lobby: dict, from_user: str, player_count: int) -> dict:
    """Find the pending invitation that should be accepted by the invited bot."""
    for invitation in lobby.get("invitations", []):
        if invitation.get("from") == from_user and invitation.get("playerCount") == player_count:
            return invitation
    raise RuntimeError(f"No invitation from {from_user} for {player_count} players.")


def create_match(client: ServerClient, players: list[str], target: int) -> None:
    """Use the lobby invitation flow to create a normal multiplayer match."""
    leader = players[0]
    player_count = len(players)

    # The first invite creates either a 2-player match or a waiting room.
    invite = client.get("/api/invite", **{"from": leader, "to": players[1], "players": player_count})
    if not invite.get("ok"):
        raise RuntimeError(f"Initial invite failed: {invite}")
    lobby = client.get("/api/lobby", username=players[1])
    invitation = find_invitation(lobby, leader, player_count)
    accepted = client.get("/api/respond", username=players[1], invite=invitation["id"], accept=1)
    if not accepted.get("ok"):
        raise RuntimeError(f"Initial invite accept failed: {accepted}")

    # For 3- and 4-player games, the leader invites extra bots into the waiting room.
    for player in players[2:]:
        leader_lobby = client.get("/api/lobby", username=leader)
        room = leader_lobby.get("waitingRoom")
        if not room:
            raise RuntimeError(f"Expected waiting room for {player_count}-player AI game.")
        invite = client.get(
            "/api/invite",
            **{"from": leader, "to": player, "players": player_count, "room": room["id"]},
        )
        if not invite.get("ok"):
            raise RuntimeError(f"Room invite failed: {invite}")
        lobby = client.get("/api/lobby", username=player)
        invitation = find_invitation(lobby, leader, player_count)
        accepted = client.get("/api/respond", username=player, invite=invitation["id"], accept=1)
        if not accepted.get("ok"):
            raise RuntimeError(f"Room invite accept failed: {accepted}")

    # The server randomly chooses who selects the match target score.
    target_lobby = client.get("/api/lobby", username=leader)
    selection = target_lobby.get("targetSelection")
    if not selection:
        raise RuntimeError(f"Target selection was not created: {target_lobby}")
    selected = client.get("/api/target", username=selection["chooser"], score=target)
    if not selected.get("ok"):
        raise RuntimeError(f"Target selection failed: {selected}")


def play_match(client: ServerClient, policy: CruceAIPolicy, players: list[str], max_actions: int = 4000) -> None:
    """Play one match by repeatedly asking the trained policy for each turn."""
    for _ in range(max_actions):
        # The leader's view is used to check whether the match still exists.
        view = client.get("/api/match", username=players[0])
        if not view.get("ok"):
            lobby = client.get("/api/lobby", username=players[0])
            if lobby.get("rematch") or not lobby.get("inMatch"):
                return
            raise RuntimeError(f"Match disappeared unexpectedly: {lobby}")

        state = view["state"]
        if state.get("status") == "Complete":
            client.get("/api/lobby", username=players[0])
            return

        actor = state.get("currentTurn")
        if not actor:
            raise RuntimeError(f"No current turn in state: {state}")

        # Fetch the current actor's private state so the model sees the correct hand.
        private_state = client.get("/api/match", username=actor)["state"]
        decision = policy.choose_action(private_state, actor)
        action = decision.get("action")

        # Submit the predicted action through the same endpoints as a human client.
        if action == "bid":
            result = client.get("/api/bid", username=actor, value=decision["value"])
        elif action == "choose_trump":
            result = client.get("/api/trump", username=actor, suit=decision["suit"])
        elif action == "play_card":
            result = client.get("/api/play", username=actor, card=decision["cardId"])
        else:
            raise RuntimeError(f"AI policy returned unsupported action: {decision}")
        if not result.get("ok"):
            raise RuntimeError(f"AI action failed: {decision} -> {result}")
    raise RuntimeError("AI-guided match exceeded action limit.")


def start_server(executable: Path, port: int) -> subprocess.Popen:
    """Start a private local server process for model-guided self-play."""
    return subprocess.Popen(
        [str(executable), str(port)],
        cwd=ROOT,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def main() -> int:
    """CLI entry point for generating games with trained Python AI models."""
    parser = argparse.ArgumentParser(description="Generate Cruce games using trained AI models.")
    parser.add_argument("--server", default="build/Debug/cruce_server_app.exe", type=Path)
    parser.add_argument("--models", default="data/ai/models/v1", type=Path)
    parser.add_argument("--games", default=15, type=int)
    parser.add_argument("--players", default=2, type=int, choices=(2, 3, 4))
    parser.add_argument("--target", default=6, type=int, choices=(6, 11, 21))
    parser.add_argument("--port", default=18181, type=int)
    parser.add_argument("--seed-name", default=str(int(time.time())))
    args = parser.parse_args()

    policy = CruceAIPolicy(args.models)
    client = ServerClient(f"http://127.0.0.1:{args.port}")
    server = start_server(args.server, args.port)
    completed = 0
    try:
        wait_for_server(client)
        for game in range(1, args.games + 1):
            # Names include the seed/mode/game/seat so generated users stay unique.
            players = [f"ai_{args.seed_name}_{args.players}_{game}_{seat}" for seat in range(args.players)]
            for player in players:
                login_or_register(client, player, player)
            create_match(client, players, args.target)
            play_match(client, policy, players)
            completed += 1
            if completed % 5 == 0 or completed == args.games:
                print(f"AI self-play progress: {completed}/{args.games}", flush=True)
    finally:
        server.terminate()
        try:
            server.wait(timeout=3)
        except subprocess.TimeoutExpired:
            server.kill()
    print(f"AI self-play complete: {completed}/{args.games} games.")
    return 0 if completed == args.games else 1


if __name__ == "__main__":
    raise SystemExit(main())
