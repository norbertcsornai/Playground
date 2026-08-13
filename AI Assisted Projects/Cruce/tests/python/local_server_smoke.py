import json
import subprocess
import sys
import time
import urllib.request
import urllib.parse
from pathlib import Path


PORT = 18080
RANK_STRENGTH = {
    "Nine": 1,
    "Jack": 2,
    "Queen": 3,
    "King": 4,
    "Ten": 5,
    "Ace": 6,
}


def get_json(path: str) -> dict:
    with urllib.request.urlopen(f"http://127.0.0.1:{PORT}{path}", timeout=3) as response:
        return json.loads(response.read().decode("utf-8"))


def enc(text: str) -> str:
    return urllib.parse.quote(text)


def wait_for_server() -> None:
    deadline = time.time() + 5
    while time.time() < deadline:
        try:
            get_json("/api/lobby?username=admin1")
            return
        except Exception:
            time.sleep(0.1)
    raise RuntimeError("local server did not become ready")


def rank(card: dict) -> str:
    return card["label"].split(" of ")[0]


def card_beats(challenger: dict, current_winner: dict, led_suit: str, trump_suit: str) -> bool:
    if challenger["suit"] == current_winner["suit"]:
        return RANK_STRENGTH[rank(challenger)] > RANK_STRENGTH[rank(current_winner)]
    if challenger["suit"] == trump_suit and current_winner["suit"] != trump_suit:
        return True
    if current_winner["suit"] == trump_suit and challenger["suit"] != trump_suit:
        return False
    return challenger["suit"] == led_suit and current_winner["suit"] != led_suit


def play_first_accepted_card(username: str) -> dict:
    state = get_json(f"/api/match?username={username}")["state"]
    for card in state["ownHand"]:
        result = get_json(f"/api/play?username={username}&card={card['id']}")
        if result["ok"]:
            return result
    raise AssertionError(f"no legal card found for {username}: {state}")


def finish_match() -> tuple[dict, dict]:
    for _ in range(1000):
        match = get_json("/api/match?username=admin1")
        if not match["ok"]:
            lobby1 = get_json("/api/lobby?username=admin1")
            lobby2 = get_json("/api/lobby?username=admin2")
            return lobby1, lobby2

        state = match["state"]
        if state["status"] == "Bidding":
            current = state["currentTurn"]
            has_bid = any(not bid["passed"] for bid in state["bids"])
            bid_value = 0 if has_bid else 1
            result = get_json(f"/api/bid?username={current}&value={bid_value}")
            assert result["ok"], result
        elif state["status"] == "ChoosingTrump":
            current = state["currentTurn"]
            player_state = get_json(f"/api/match?username={current}")["state"]
            suit = player_state["ownHand"][0]["suit"]
            result = get_json(f"/api/trump?username={current}&suit={suit}")
            assert result["ok"], result
        elif state["status"] == "Playing":
            result = play_first_accepted_card(state["currentTurn"])
            assert result["ok"], result
        elif state["status"] == "Complete":
            lobby1 = get_json("/api/lobby?username=admin1")
            lobby2 = get_json("/api/lobby?username=admin2")
            return lobby1, lobby2

    raise AssertionError("match did not finish within the action limit")


def login_or_register(username: str, password: str, platform: str = "Web") -> dict:
    get_json(
        f"/api/register?username={enc(username)}&password={enc(password)}&platform={platform}"
    )
    login = get_json(
        f"/api/login?username={enc(username)}&password={enc(password)}&platform={platform}"
    )
    assert login["ok"], login
    return login


def find_invitation(lobby: dict, from_user: str, player_count: int) -> dict:
    for invitation in lobby["invitations"]:
        if invitation["from"] == from_user and invitation["playerCount"] == player_count:
            return invitation
    raise AssertionError(f"expected invitation from {from_user} for {player_count} players: {lobby}")


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: local_server_smoke.py <cruce_server_app>", file=sys.stderr)
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

        login1 = get_json("/api/login?username=admin1&password=admin1&platform=Web")
        login2 = get_json("/api/login?username=admin2&password=admin2&platform=WindowsDesktop")
        invite = get_json("/api/invite?from=admin1&to=admin2")
        accept = get_json("/api/respond?username=admin2&invite=1&accept=1")
        target_lobby = get_json("/api/lobby?username=admin1")
        target_selection = target_lobby["targetSelection"]
        target = get_json(f"/api/target?username={target_selection['chooser']}&score=6")
        lobby = get_json("/api/lobby?username=admin1")
        match1 = get_json("/api/match?username=admin1")
        match2 = get_json("/api/match?username=admin2")

        assert login1["ok"], login1
        assert login2["ok"], login2
        assert invite["ok"], invite
        assert accept["ok"], accept
        assert target_lobby["ok"], target_lobby
        assert not target_lobby["inMatch"], target_lobby
        assert target_selection["chooser"] in {"admin1", "admin2"}, target_lobby
        assert target["ok"], target
        assert lobby["ok"], lobby
        assert lobby["inMatch"], lobby
        assert lobby["matchId"] == "match-1", lobby
        assert match1["ok"], match1
        assert match2["ok"], match2
        assert match1["state"]["targetScore"] == 6, match1
        assert len(match1["state"]["ownHand"]) == 8, match1
        assert len(match2["state"]["ownHand"]) == 8, match2
        assert match1["state"]["scores"] == {"admin1": 0, "admin2": 0}, match1
        assert match1["state"]["roundPoints"] == {"admin1": 0, "admin2": 0}, match1
        first_bidder = match1["state"]["currentTurn"]
        assert first_bidder in {"admin1", "admin2"}, match1
        second_bidder = "admin1" if first_bidder == "admin2" else "admin2"

        bid = get_json(f"/api/bid?username={first_bidder}&value=1")
        waiting_player_view = get_json(f"/api/match?username={second_bidder}")
        waiting = get_json(f"/api/bid?username={first_bidder}&value=2")
        bid_pass = get_json(f"/api/bid?username={second_bidder}&value=0")
        after_bids = get_json(f"/api/match?username={first_bidder}")
        trump_suit = after_bids["state"]["ownHand"][0]["suit"]
        trump = get_json(f"/api/trump?username={first_bidder}&suit={trump_suit}")
        after_trump = get_json(f"/api/match?username={first_bidder}")
        first_card = after_trump["state"]["ownHand"][0]
        play = get_json(f"/api/play?username={first_bidder}&card={first_card['id']}")
        other_view_after_play = get_json(f"/api/match?username={second_bidder}")
        second_card = other_view_after_play["state"]["ownHand"][0]
        second_play = get_json(f"/api/play?username={second_bidder}&card={second_card['id']}")
        after_trick = get_json(f"/api/match?username={first_bidder}")
        expected_winner = (
            second_bidder
            if card_beats(second_card, first_card, first_card["suit"], trump_suit)
            else first_bidder
        )

        assert bid["ok"], bid
        assert waiting_player_view["state"]["currentTurn"] == second_bidder, waiting_player_view
        assert waiting_player_view["state"]["bids"][-1]["player"] == first_bidder, waiting_player_view
        assert waiting_player_view["state"]["bids"][-1]["value"] == 1, waiting_player_view
        assert not waiting["ok"], waiting
        assert bid_pass["ok"], bid_pass
        assert after_bids["state"]["status"] == "ChoosingTrump", after_bids
        assert after_bids["state"]["currentTurn"] == first_bidder, after_bids
        assert trump["ok"], trump
        assert after_trump["state"]["status"] == "Playing", after_trump
        assert after_trump["state"]["currentTurn"] == first_bidder, after_trump
        assert play["ok"], play
        assert len(play["state"]["ownHand"]) == 7, play
        assert other_view_after_play["state"]["currentTrick"][0]["card"]["id"] == first_card["id"], other_view_after_play
        assert second_play["ok"], second_play
        assert after_trick["state"]["currentTrick"] == [], after_trick
        assert after_trick["state"]["lastTrickWinner"] == expected_winner, after_trick
        assert after_trick["state"]["currentTurn"] == expected_winner, after_trick
        assert sum(after_trick["state"]["roundPoints"].values()) >= (
            first_card["points"] + second_card["points"]
        ), after_trick
        assert after_trick["state"]["roundResults"] == [], after_trick

        complete_lobby1, complete_lobby2 = finish_match()
        assert complete_lobby1["ok"], complete_lobby1
        assert complete_lobby2["ok"], complete_lobby2
        assert not complete_lobby1["inMatch"], complete_lobby1
        assert not complete_lobby2["inMatch"], complete_lobby2
        assert "won the match" in complete_lobby1["notice"], complete_lobby1
        assert sorted(complete_lobby1["rematch"]["players"]) == ["admin1", "admin2"], complete_lobby1
        assert not complete_lobby1["rematch"]["responded"], complete_lobby1
        assert complete_lobby1["notice"] == complete_lobby2["notice"], (
            complete_lobby1,
            complete_lobby2,
        )
        accept_rematch1 = get_json("/api/rematch?username=admin1&accept=1")
        rematch_waiting = get_json("/api/lobby?username=admin2")
        accept_rematch2 = get_json("/api/rematch?username=admin2&accept=1")
        rematch_target_lobby = get_json("/api/lobby?username=admin1")
        rematch_target = rematch_target_lobby["targetSelection"]
        start_rematch = get_json(f"/api/target?username={rematch_target['chooser']}&score=6")
        rematch_match = get_json("/api/match?username=admin1")

        assert accept_rematch1["ok"], accept_rematch1
        assert rematch_waiting["rematch"]["responses"] == {"admin1": True}, rematch_waiting
        assert accept_rematch2["ok"], accept_rematch2
        assert rematch_target["playerCount"] == 2, rematch_target_lobby
        assert sorted(rematch_target["players"]) == ["admin1", "admin2"], rematch_target
        assert start_rematch["ok"], start_rematch
        assert rematch_match["ok"], rematch_match
        assert len(rematch_match["state"]["players"]) == 2, rematch_match

        complete_lobby1, complete_lobby2 = finish_match()
        assert sorted(complete_lobby1["rematch"]["players"]) == ["admin1", "admin2"], complete_lobby1
        decline_rematch = get_json("/api/rematch?username=admin2&accept=0")
        declined_lobby1 = get_json("/api/lobby?username=admin1")
        declined_lobby2 = get_json("/api/lobby?username=admin2")
        assert decline_rematch["ok"], decline_rematch
        assert declined_lobby1["rematch"] is None, declined_lobby1
        assert declined_lobby2["rematch"] is None, declined_lobby2
        assert "declined the rematch" in declined_lobby1["notice"], declined_lobby1

        login_or_register("admin3", "admin3")
        grouped_invite2 = get_json("/api/invite?from=admin1&to=admin2&players=3")
        grouped_invite3 = get_json("/api/invite?from=admin1&to=admin3&players=3")
        grouped_lobby2 = get_json("/api/lobby?username=admin2")
        grouped_accept2 = get_json(
            f"/api/respond?username=admin2&invite={find_invitation(grouped_lobby2, 'admin1', 3)['id']}&accept=1"
        )
        grouped_waiting = get_json("/api/lobby?username=admin1")["waitingRoom"]
        grouped_lobby3 = get_json("/api/lobby?username=admin3")
        grouped_invitation3 = find_invitation(grouped_lobby3, "admin1", 3)
        grouped_accept3 = get_json(
            f"/api/respond?username=admin3&invite={grouped_invitation3['id']}&accept=1"
        )
        grouped_target_lobby = get_json("/api/lobby?username=admin1")
        grouped_target = grouped_target_lobby["targetSelection"]
        grouped_cancel = get_json("/api/cancel?username=admin1")

        assert grouped_invite2["ok"], grouped_invite2
        assert grouped_invite3["ok"], grouped_invite3
        assert grouped_accept2["ok"], grouped_accept2
        assert grouped_waiting["playerCount"] == 3, grouped_waiting
        assert grouped_waiting["players"] == ["admin1", "admin2"], grouped_waiting
        assert grouped_invitation3["waitingRoomId"] == grouped_waiting["id"], grouped_lobby3
        assert grouped_accept3["ok"], grouped_accept3
        assert grouped_target["playerCount"] == 3, grouped_target_lobby
        assert sorted(grouped_target["players"]) == ["admin1", "admin2", "admin3"], grouped_target
        assert grouped_cancel["ok"], grouped_cancel

        cancel_invite = get_json("/api/invite?from=admin1&to=admin2&players=4")
        cancel_invite_lobby = get_json("/api/lobby?username=admin2")
        cancel_accept = get_json(
            f"/api/respond?username=admin2&invite={find_invitation(cancel_invite_lobby, 'admin1', 4)['id']}&accept=1"
        )
        cancel_waiting_lobby = get_json("/api/lobby?username=admin1")
        cancel_waiting = get_json("/api/cancel?username=admin2")
        canceled_lobby1 = get_json("/api/lobby?username=admin1")
        canceled_lobby2 = get_json("/api/lobby?username=admin2")
        assert cancel_invite["ok"], cancel_invite
        assert cancel_accept["ok"], cancel_accept
        assert cancel_waiting_lobby["waitingRoom"]["playerCount"] == 4, cancel_waiting_lobby
        assert cancel_waiting["ok"], cancel_waiting
        assert canceled_lobby1["waitingRoom"] is None, canceled_lobby1
        assert canceled_lobby2["waitingRoom"] is None, canceled_lobby2
        assert not canceled_lobby1["inMatch"], canceled_lobby1
        assert "game was canceled" in canceled_lobby1["notice"], canceled_lobby1

        invite3 = get_json("/api/invite?from=admin1&to=admin2&players=3")
        invite3_lobby = get_json("/api/lobby?username=admin2")
        accept3 = get_json(
            f"/api/respond?username=admin2&invite={find_invitation(invite3_lobby, 'admin1', 3)['id']}&accept=1"
        )
        waiting_lobby = get_json("/api/lobby?username=admin1")
        waiting_room = waiting_lobby["waitingRoom"]
        chat = get_json("/api/chat?username=admin1&message=ready")
        chat_lobby = get_json("/api/lobby?username=admin2")
        room_invite = get_json(
            f"/api/invite?from=admin1&to=admin3&players=3&room={waiting_room['id']}"
        )
        admin3_lobby = get_json("/api/lobby?username=admin3")
        room_accept = get_json(
            f"/api/respond?username=admin3&invite={find_invitation(admin3_lobby, 'admin1', 3)['id']}&accept=1"
        )
        target3_lobby = get_json("/api/lobby?username=admin1")
        target3 = target3_lobby["targetSelection"]
        start3 = get_json(f"/api/target?username={target3['chooser']}&score=6")
        match3 = get_json("/api/match?username=admin1")

        assert invite3["ok"], invite3
        assert accept3["ok"], accept3
        assert waiting_room["playerCount"] == 3, waiting_lobby
        assert waiting_room["players"] == ["admin1", "admin2"], waiting_lobby
        assert chat["ok"], chat
        assert chat_lobby["waitingRoom"]["messages"][-1] == {
            "from": "admin1",
            "message": "ready",
        }, chat_lobby
        assert room_invite["ok"], room_invite
        assert room_accept["ok"], room_accept
        assert target3["playerCount"] == 3, target3_lobby
        assert sorted(target3["players"]) == ["admin1", "admin2", "admin3"], target3
        assert start3["ok"], start3
        assert match3["ok"], match3
        assert len(match3["state"]["players"]) == 3, match3
        assert len(match3["state"]["ownHand"]) == 8, match3
        assert set(match3["state"]["scores"]) == {"admin1", "admin2", "admin3"}, match3
        cancel_match = get_json("/api/cancel?username=admin3")
        after_cancel1 = get_json("/api/lobby?username=admin1")
        after_cancel2 = get_json("/api/lobby?username=admin2")
        after_cancel3 = get_json("/api/lobby?username=admin3")
        assert cancel_match["ok"], cancel_match
        assert not after_cancel1["inMatch"], after_cancel1
        assert not after_cancel2["inMatch"], after_cancel2
        assert not after_cancel3["inMatch"], after_cancel3
        assert "game was canceled" in after_cancel1["notice"], after_cancel1

        login_or_register("admin4", "admin4")
        invite4_2 = get_json("/api/invite?from=admin1&to=admin2&players=4")
        invite4_3 = get_json("/api/invite?from=admin1&to=admin3&players=4")
        invite4_4 = get_json("/api/invite?from=admin1&to=admin4&players=4")
        lobby4_2 = get_json("/api/lobby?username=admin2")
        accept4_2 = get_json(
            f"/api/respond?username=admin2&invite={find_invitation(lobby4_2, 'admin1', 4)['id']}&accept=1"
        )
        lobby4_3 = get_json("/api/lobby?username=admin3")
        accept4_3 = get_json(
            f"/api/respond?username=admin3&invite={find_invitation(lobby4_3, 'admin1', 4)['id']}&accept=1"
        )
        lobby4_4 = get_json("/api/lobby?username=admin4")
        accept4_4 = get_json(
            f"/api/respond?username=admin4&invite={find_invitation(lobby4_4, 'admin1', 4)['id']}&accept=1"
        )
        target4_lobby = get_json("/api/lobby?username=admin1")
        target4 = target4_lobby["targetSelection"]
        start4 = get_json(f"/api/target?username={target4['chooser']}&score=6")
        match4 = get_json("/api/match?username=admin1")
        team_labels = sorted(player.get("team") for player in match4["state"]["players"])
        cancel4 = get_json("/api/cancel?username=admin1")

        assert invite4_2["ok"], invite4_2
        assert invite4_3["ok"], invite4_3
        assert invite4_4["ok"], invite4_4
        assert accept4_2["ok"], accept4_2
        assert accept4_3["ok"], accept4_3
        assert accept4_4["ok"], accept4_4
        assert target4["playerCount"] == 4, target4_lobby
        assert start4["ok"], start4
        assert match4["ok"], match4
        assert len(match4["state"]["players"]) == 4, match4
        assert team_labels == ["Team 1", "Team 1", "Team 2", "Team 2"], match4
        assert cancel4["ok"], cancel4

        print("[PASS] local server login/invite smoke")
        return 0
    finally:
        server.terminate()
        try:
            server.wait(timeout=3)
        except subprocess.TimeoutExpired:
            server.kill()


if __name__ == "__main__":
    raise SystemExit(main())
