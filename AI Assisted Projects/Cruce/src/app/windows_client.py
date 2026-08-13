from __future__ import annotations

import json
import math
import sys
import urllib.parse
import urllib.request
from pathlib import Path
import tkinter as tk
from tkinter import ttk

try:
    import winsound
except ImportError:  # pragma: no cover - Windows client fallback for non-Windows linting.
    winsound = None


SERVER_URL = "http://127.0.0.1:8080"
POLL_MS = 2000
REPO_ROOT = Path(__file__).resolve().parents[2]
SESSION_FILE = REPO_ROOT / "data" / "windows_session.txt"

APP_BG = "#f4f7f5"
SURFACE = "#ffffff"
SURFACE_ALT = "#f8faf9"
HEADER = "#153c35"
INK = "#1b2b28"
MUTED = "#5f716c"
LINE = "#d7ded9"
ACCENT = "#a84f2b"
ACCENT_DARK = "#843d23"
GOLD = "#dfb64c"
SUCCESS = "#145331"
SUCCESS_BG = "#e2f4e9"
WAITING = "#704b12"
WAITING_BG = "#fff0c4"
ERROR = "#9f1d1d"
ERROR_BG = "#fde8e8"


def request_json(path: str, **params: object) -> dict:
    query = urllib.parse.urlencode(params)
    url = f"{SERVER_URL}{path}"
    if query:
        url = f"{url}?{query}"
    try:
        with urllib.request.urlopen(url, timeout=4) as response:
            return json.loads(response.read().decode("utf-8"))
    except Exception as exc:
        return {"ok": False, "message": f"Cannot reach Cruce server: {exc}"}


def action_text(status: str) -> str:
    if status == "Bidding":
        return "select a bid or accept/pass"
    if status == "ChoosingTrump":
        return "choose trump"
    if status == "Playing":
        return "play a legal card"
    return "wait for the next round"


def owner_label(owner: str) -> str:
    if owner.startswith("team-"):
        try:
            return f"Team {int(owner.removeprefix('team-')) + 1}"
        except ValueError:
            return owner
    return owner


def team_text(player: dict, state: dict) -> str:
    if len(state.get("players", [])) == 4 and player.get("team"):
        return f" - {player['team']}"
    return ""


class ImageCache:
    def __init__(self) -> None:
        self._images: dict[tuple[str, int, int], tk.PhotoImage] = {}

    def get(self, image_path: str, max_width: int = 92, max_height: int = 132) -> tk.PhotoImage | None:
        key = (image_path, max_width, max_height)
        if key in self._images:
            return self._images[key]

        local_path = REPO_ROOT / image_path.lstrip("/")
        if not local_path.exists():
            return None

        try:
            image = tk.PhotoImage(file=str(local_path))
            factor = max(
                1,
                math.ceil(image.width() / max_width),
                math.ceil(image.height() / max_height),
            )
            if factor > 1:
                image = image.subsample(factor, factor)
            self._images[key] = image
            return image
        except tk.TclError:
            return None


class CardGrid(ttk.Frame):
    def __init__(
        self,
        parent: tk.Widget,
        images: ImageCache,
        empty_text: str,
        selectable: bool = False,
        on_select=None,
        on_double=None,
    ) -> None:
        super().__init__(parent)
        self.images = images
        self.empty_text = empty_text
        self.selectable = selectable
        self.on_select = on_select
        self.on_double = on_double
        self.cards: list[dict] = []
        self.selected_id: int | None = None
        self._render_after: str | None = None
        self._rendered_signature: tuple | None = None
        self._rendered_columns = 0
        self._cells_by_id: dict[int, tuple[tk.Frame, tk.Label, tk.Label | None]] = {}
        self.bind("<Configure>", self._queue_render)

    def request_render(self) -> None:
        self._queue_render()

    def set_cards(self, cards: list[dict], selected_id: int | None = None) -> None:
        self.cards = cards
        previous_selected = self.selected_id
        self.selected_id = selected_id
        signature = self._card_signature()
        columns = self._columns()
        if signature == self._rendered_signature and columns == self._rendered_columns:
            if previous_selected != self.selected_id:
                self._update_selection_visuals()
            return
        self._render()

    def _queue_render(self, _event=None) -> None:
        if self._render_after is not None:
            self.after_cancel(self._render_after)
        self._render_after = self.after(80, self._render)

    def _render(self) -> None:
        self._render_after = None
        signature = self._card_signature()
        columns = self._columns()
        if signature == self._rendered_signature and columns == self._rendered_columns:
            self._update_selection_visuals()
            return

        for child in self.winfo_children():
            child.destroy()
        self._cells_by_id.clear()

        if not self.cards:
            ttk.Label(self, text=self.empty_text).grid(row=0, column=0, sticky="w", padx=8, pady=8)
            self._rendered_signature = signature
            self._rendered_columns = columns
            return

        for index, card in enumerate(self.cards):
            row = index // columns
            column = index % columns
            selected = self.selectable and card.get("id") == self.selected_id and bool(card.get("_legal", True))
            legal = bool(card.get("_legal", True))
            marked_play_state = self.selectable and "_legal" in card
            background, border, thickness = self._style_for(selected, legal, marked_play_state)

            cell = tk.Frame(
                self,
                bg=background,
                highlightbackground=border,
                highlightcolor=border,
                highlightthickness=thickness,
                padx=4,
                pady=4,
                cursor="hand2" if marked_play_state and legal else "",
            )
            cell.grid(row=row, column=column, sticky="nsew", padx=5, pady=5)
            self.columnconfigure(column, weight=1)

            image = self.images.get(card.get("image", ""))
            if image is not None:
                image_label = tk.Label(
                    cell,
                    image=image,
                    bg=background,
                    state="normal" if legal else "disabled",
                    cursor="hand2" if marked_play_state and legal else "",
                )
                image_label.image = image
            else:
                image_label = tk.Label(
                    cell,
                    text=card.get("label", "Card"),
                    bg=background,
                    fg=INK if legal else MUTED,
                    width=14,
                    height=7,
                    cursor="hand2" if marked_play_state and legal else "",
                )
            image_label.pack(fill="both", expand=True)

            title = card.get("label", "")
            if card.get("player"):
                title = f"{card['player']}: {title}"
            label = tk.Label(
                cell,
                text=title,
                bg=background,
                fg=INK if legal else MUTED,
                wraplength=108,
                justify="center",
                font=("Segoe UI", 9),
                cursor="hand2" if marked_play_state and legal else "",
            )
            label.pack(fill="x")
            if isinstance(card.get("id"), int):
                self._cells_by_id[card["id"]] = (cell, image_label, label)

            for widget in (cell, image_label, label):
                widget.bind("<Button-1>", lambda _event, c=card: self._select(c))
                widget.bind("<Double-Button-1>", lambda _event, c=card: self._double(c))

        self._rendered_signature = signature
        self._rendered_columns = columns

    def _select(self, card: dict) -> None:
        if not self.selectable:
            return
        if not card.get("_legal", True):
            if self.on_select:
                self.on_select(card)
            return
        self.selected_id = card.get("id")
        if self.on_select:
            self.on_select(card)
        self._update_selection_visuals()

    def _double(self, card: dict) -> None:
        if self.selectable and not card.get("_legal", True):
            if self.on_select:
                self.on_select(card)
            return
        if self.selectable:
            self._select(card)
        if self.on_double:
            self.on_double(card)

    def _columns(self) -> int:
        return max(1, max(1, self.winfo_width()) // 126)

    def _card_signature(self) -> tuple:
        return tuple(
            (
                card.get("id"),
                card.get("label"),
                card.get("image"),
                card.get("player"),
                card.get("_legal", True),
            )
            for card in self.cards
        )

    @staticmethod
    def _style_for(selected: bool, legal: bool = True, marked_play_state: bool = False) -> tuple[str, str, int]:
        if marked_play_state and not legal:
            return ("#eef2ef", "#d5ddd8", 1)
        if selected:
            return (SUCCESS_BG, SUCCESS, 3)
        if marked_play_state and legal:
            return ("#f0fff5", SUCCESS, 2)
        return ("#fffdf9", "#c9d1cc", 1)

    def _update_selection_visuals(self) -> None:
        if not self.selectable:
            return
        for card_id, widgets in self._cells_by_id.items():
            selected = card_id == self.selected_id
            card = next((item for item in self.cards if item.get("id") == card_id), {})
            legal = bool(card.get("_legal", True))
            marked_play_state = self.selectable and "_legal" in card
            background, border, thickness = self._style_for(selected and legal, legal, marked_play_state)
            cell, image_label, label = widgets
            cell.configure(
                bg=background,
                highlightbackground=border,
                highlightcolor=border,
                highlightthickness=thickness,
                cursor="hand2" if marked_play_state and legal else "",
            )
            image_label.configure(
                bg=background,
                state="normal" if legal else "disabled",
                cursor="hand2" if marked_play_state and legal else "",
            )
            if label is not None:
                label.configure(
                    bg=background,
                    fg=INK if legal else MUTED,
                    cursor="hand2" if marked_play_state and legal else "",
                )


class CruceWindowsClient:
    def __init__(self) -> None:
        self.root = tk.Tk()
        self.root.title("Cruce Windows Client")
        self.root.geometry("1360x880")
        self.root.minsize(1060, 720)

        self.current_user = ""
        self.waiting_room: dict | None = None
        self.rematch_offer: dict | None = None
        self.selected_card_id: int | None = None
        self.last_match_state: dict | None = None
        self.last_turn_key = ""
        self.last_trick_key = ""
        self.last_winner_key = ""
        self.last_invite_count = 0
        self.images = ImageCache()

        self._configure_styles()
        self._build_ui()
        self._show_logged_out()
        self._try_restore_session()
        self.root.after(POLL_MS, self._poll)

    def run(self) -> None:
        self.root.mainloop()

    def _configure_styles(self) -> None:
        self.root.configure(bg=APP_BG)
        style = ttk.Style()
        style.theme_use("clam")
        self.root.option_add("*Font", ("Segoe UI", 10))
        style.configure("TFrame", background=APP_BG)
        style.configure("TLabel", background=APP_BG, foreground=INK)
        style.configure("TButton", font=("Segoe UI", 10, "bold"), padding=(10, 6), background=ACCENT, foreground="#ffffff", borderwidth=0)
        style.map(
            "TButton",
            background=[("disabled", "#cfd9d4"), ("active", ACCENT_DARK), ("pressed", ACCENT_DARK)],
            foreground=[("disabled", "#66756f"), ("active", "#ffffff"), ("pressed", "#ffffff")],
        )
        style.configure("TEntry", fieldbackground=SURFACE, foreground=INK, bordercolor=LINE, lightcolor=LINE, darkcolor=LINE, padding=5)
        style.configure("TCombobox", fieldbackground=SURFACE, foreground=INK, background=SURFACE, bordercolor=LINE, arrowcolor=HEADER)
        style.configure("Panel.TLabelframe", background=APP_BG, bordercolor=LINE, relief="solid")
        style.configure("Panel.TLabelframe.Label", background=APP_BG, foreground=HEADER, font=("Segoe UI", 11, "bold"))
        style.configure("Inner.TLabelframe", background=SURFACE, bordercolor=LINE)
        style.configure("Inner.TLabelframe.Label", background=SURFACE, foreground=HEADER, font=("Segoe UI", 10, "bold"))
        style.configure("Banner.TLabel", font=("Segoe UI", 11, "bold"), padding=10)
        style.configure("Ready.TLabel", background=SUCCESS_BG, foreground=SUCCESS)
        style.configure("Waiting.TLabel", background=WAITING_BG, foreground=WAITING)
        style.configure("Status.TLabel", background="#e8efeb", foreground=SUCCESS, padding=(12, 8), font=("Segoe UI", 10, "bold"))
        style.configure("Treeview", background=SURFACE, fieldbackground=SURFACE, foreground=INK, rowheight=30, bordercolor=LINE)
        style.configure("Treeview.Heading", background="#e8efeb", foreground=HEADER, font=("Segoe UI", 10, "bold"))
        style.map("Treeview", background=[("selected", SUCCESS_BG)], foreground=[("selected", INK)])

    def _build_ui(self) -> None:
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(3, weight=1)

        self.header = tk.Frame(self.root, bg=HEADER, padx=18, pady=12)
        self.header.grid(row=0, column=0, sticky="ew")
        self.header.columnconfigure(0, weight=1)
        tk.Label(
            self.header,
            text="Cruce",
            bg=HEADER,
            fg="#ffffff",
            font=("Segoe UI", 20, "bold"),
            anchor="w",
        ).grid(row=0, column=0, sticky="w")
        tk.Frame(self.header, bg=GOLD, height=4).grid(row=1, column=0, sticky="ew", pady=(10, 0))

        self.auth_frame = ttk.Frame(self.root, padding=(14, 12))
        self.auth_frame.grid(row=1, column=0, sticky="ew")
        self.auth_frame.columnconfigure(5, weight=1)
        ttk.Label(self.auth_frame, text="Username").grid(row=0, column=0, padx=(0, 6))
        self.username = ttk.Entry(self.auth_frame, width=18)
        self.username.grid(row=0, column=1, padx=(0, 12))
        ttk.Label(self.auth_frame, text="Password").grid(row=0, column=2, padx=(0, 6))
        self.password = ttk.Entry(self.auth_frame, show="*", width=18)
        self.password.grid(row=0, column=3, padx=(0, 12))
        ttk.Button(self.auth_frame, text="Login", command=lambda: self._authenticate(False)).grid(row=0, column=4, padx=4)
        ttk.Button(self.auth_frame, text="Register", command=lambda: self._authenticate(True)).grid(row=0, column=5, sticky="w", padx=4)

        self.message = ttk.Label(
            self.root,
            text="Welcome back.",
            anchor="w",
            style="Status.TLabel",
        )
        self.message.grid(row=2, column=0, sticky="ew", padx=14, pady=(0, 8))

        self.content = ttk.Frame(self.root, padding=(14, 0, 14, 14))
        self.content.grid(row=3, column=0, sticky="nsew")
        self.content.columnconfigure(0, weight=1)
        self.content.rowconfigure(3, weight=1)

        self._build_lobby()
        self._build_waiting_room()
        self._build_rematch()
        self._build_target()
        self._build_match()

    def _build_lobby(self) -> None:
        self.lobby_frame = ttk.LabelFrame(self.content, text="Online Players", style="Panel.TLabelframe", padding=12)
        self.lobby_frame.grid(row=0, column=0, sticky="ew")
        self.lobby_frame.columnconfigure(0, weight=2)
        self.lobby_frame.columnconfigure(1, weight=1)

        controls = ttk.Frame(self.lobby_frame)
        controls.grid(row=0, column=0, columnspan=2, sticky="ew", pady=(0, 8))
        ttk.Button(controls, text="Refresh", command=self.refresh_lobby).pack(side="left", padx=(0, 6))
        self.invite_button = ttk.Button(controls, text="Invite", command=self.invite_selected_player)
        self.invite_button.pack(side="left", padx=(0, 14))
        self.game_size_label = ttk.Label(controls, text="Invite to")
        self.game_size_label.pack(side="left", padx=(0, 6))
        self.game_size = ttk.Combobox(controls, values=["2 players", "3 players", "4 players"], width=10, state="readonly")
        self.game_size.current(0)
        self.game_size.pack(side="left")

        online_frame = ttk.Frame(self.lobby_frame)
        online_frame.grid(row=1, column=0, sticky="nsew", padx=(0, 8))
        online_frame.columnconfigure(0, weight=1)
        self.online = ttk.Treeview(online_frame, columns=("platform", "status", "wins"), show="tree headings", height=6)
        self.online.heading("#0", text="Username")
        self.online.heading("platform", text="Platform")
        self.online.heading("status", text="Status")
        self.online.heading("wins", text="Wins")
        self.online.column("#0", width=160)
        self.online.column("platform", width=120)
        self.online.column("status", width=90)
        self.online.column("wins", width=70, anchor="center")
        self.online.grid(row=0, column=0, sticky="nsew")

        invite_frame = ttk.Frame(self.lobby_frame)
        invite_frame.grid(row=1, column=1, sticky="nsew")
        invite_frame.columnconfigure(0, weight=1)
        invite_frame.rowconfigure(0, weight=1)
        online_frame.rowconfigure(0, weight=1)
        self.invitations = ttk.Treeview(invite_frame, columns=("size",), show="tree headings", height=6)
        self.invitations.heading("#0", text="From")
        self.invitations.heading("size", text="Game")
        self.invitations.grid(row=0, column=0, sticky="nsew")
        invite_buttons = ttk.Frame(invite_frame)
        invite_buttons.grid(row=1, column=0, sticky="ew", pady=(6, 0))
        ttk.Button(invite_buttons, text="Accept", command=lambda: self.respond_invitation(True)).pack(side="left", padx=(0, 6))
        ttk.Button(invite_buttons, text="Decline", command=lambda: self.respond_invitation(False)).pack(side="left")

        profile_frame = ttk.LabelFrame(self.lobby_frame, text="Your Profile", style="Inner.TLabelframe", padding=8)
        profile_frame.grid(row=2, column=0, columnspan=2, sticky="ew", pady=(10, 0))
        profile_frame.columnconfigure(1, weight=1)
        self.profile_avatar = tk.Label(
            profile_frame,
            text="?",
            bg=HEADER,
            fg="#ffffff",
            width=4,
            height=2,
            font=("Segoe UI", 14, "bold"),
        )
        self.profile_avatar.grid(row=0, column=0, sticky="nw", padx=(0, 10))
        self.profile_text = ttk.Label(profile_frame, text="No profile loaded.", anchor="w", justify="left")
        self.profile_text.grid(row=0, column=1, sticky="ew")

    def _build_waiting_room(self) -> None:
        self.waiting_frame = ttk.LabelFrame(self.content, text="Waiting Room", style="Panel.TLabelframe", padding=12)
        self.waiting_frame.grid(row=1, column=0, sticky="ew", pady=8)
        self.waiting_frame.columnconfigure(0, weight=1)
        self.waiting_status = ttk.Label(self.waiting_frame, style="Waiting.TLabel", anchor="w")
        self.waiting_status.grid(row=0, column=0, sticky="ew", pady=(0, 6))
        self.waiting_players = ttk.Label(self.waiting_frame, anchor="w")
        self.waiting_players.grid(row=1, column=0, sticky="ew", pady=(0, 6))
        self.chat = tk.Text(self.waiting_frame, height=5, wrap="word", state="disabled", relief="flat", padx=10, pady=8)
        self.chat.configure(bg=SURFACE_ALT, fg=INK, insertbackground=INK, highlightthickness=1, highlightbackground=LINE)
        self.chat.grid(row=2, column=0, sticky="ew", pady=(0, 6))
        chat_row = ttk.Frame(self.waiting_frame)
        chat_row.grid(row=3, column=0, sticky="ew")
        chat_row.columnconfigure(0, weight=1)
        self.chat_entry = ttk.Entry(chat_row)
        self.chat_entry.grid(row=0, column=0, sticky="ew", padx=(0, 6))
        self.chat_entry.bind("<Return>", lambda _event: self.send_chat())
        ttk.Button(chat_row, text="Send", command=self.send_chat).grid(row=0, column=1, padx=(0, 6))
        ttk.Button(chat_row, text="Return to Lobby", command=self.return_to_lobby).grid(row=0, column=2)

    def _build_rematch(self) -> None:
        self.rematch_frame = ttk.LabelFrame(self.content, text="Rematch", style="Panel.TLabelframe", padding=12)
        self.rematch_frame.grid(row=2, column=0, sticky="ew", pady=(0, 8))
        self.rematch_frame.columnconfigure(0, weight=1)
        self.rematch_notice = ttk.Label(self.rematch_frame, style="Waiting.TLabel", anchor="w")
        self.rematch_notice.grid(row=0, column=0, sticky="ew", pady=(0, 8))
        self.rematch_responses = ttk.Label(self.rematch_frame, anchor="w")
        self.rematch_responses.grid(row=1, column=0, sticky="ew", pady=(0, 8))
        buttons = ttk.Frame(self.rematch_frame)
        buttons.grid(row=2, column=0, sticky="w")
        self.rematch_yes = ttk.Button(buttons, text="Yes", command=lambda: self.respond_rematch(True))
        self.rematch_yes.pack(side="left", padx=(0, 6))
        self.rematch_no = ttk.Button(buttons, text="No", command=lambda: self.respond_rematch(False))
        self.rematch_no.pack(side="left", padx=(0, 6))
        ttk.Button(buttons, text="Return to Lobby", command=self.return_to_lobby).pack(side="left")

    def _build_target(self) -> None:
        self.target_frame = ttk.LabelFrame(self.content, text="Target Score", style="Panel.TLabelframe", padding=12)
        self.target_frame.grid(row=2, column=0, sticky="ew", pady=(0, 8))
        self.target_frame.columnconfigure(0, weight=1)
        self.target_notice = ttk.Label(self.target_frame, style="Waiting.TLabel", anchor="w")
        self.target_notice.grid(row=0, column=0, sticky="ew", pady=(0, 8))
        buttons = ttk.Frame(self.target_frame)
        buttons.grid(row=1, column=0, sticky="w")
        self.target_buttons = []
        for score in (6, 11, 21):
            button = ttk.Button(buttons, text=f"{score} points", command=lambda s=score: self.select_target(s))
            button.pack(side="left", padx=(0, 6))
            self.target_buttons.append(button)
        ttk.Button(buttons, text="Return to Lobby", command=self.return_to_lobby).pack(side="left")

    def _build_match(self) -> None:
        self.match_frame = ttk.LabelFrame(self.content, text="Match", style="Panel.TLabelframe", padding=12)
        self.match_frame.grid(row=3, column=0, sticky="nsew")
        self.match_frame.columnconfigure(0, weight=2)
        self.match_frame.columnconfigure(1, weight=3)
        self.match_frame.rowconfigure(2, weight=1)

        self.turn_notice = ttk.Label(self.match_frame, style="Waiting.TLabel", anchor="w")
        self.turn_notice.grid(row=0, column=0, sticky="ew", pady=(0, 6), padx=(0, 8))
        ttk.Button(self.match_frame, text="Return to Lobby", command=self.return_to_lobby).grid(
            row=0,
            column=1,
            sticky="e",
            pady=(0, 6),
        )
        self.match_message = ttk.Label(self.match_frame, text="", anchor="w")
        self.match_message.grid(row=1, column=0, columnspan=2, sticky="ew", pady=(0, 6))

        left = ttk.Frame(self.match_frame)
        left.grid(row=2, column=0, sticky="nsew", padx=(0, 8))
        left.columnconfigure(0, weight=1)
        left.rowconfigure(1, weight=1)

        table_box = ttk.LabelFrame(left, text="Table Seats", style="Inner.TLabelframe", padding=8)
        table_box.grid(row=0, column=0, sticky="ew", pady=(0, 8))
        for index in range(3):
            table_box.columnconfigure(index, weight=1)
            table_box.rowconfigure(index, weight=1)
        self.seat_labels: dict[int, tk.Label] = {}
        for seat, row, column in ((2, 0, 1), (1, 1, 0), (3, 1, 2), (0, 2, 1)):
            label = tk.Label(
                table_box,
                text="",
                bg=SURFACE,
                fg=INK,
                relief="solid",
                bd=1,
                padx=8,
                pady=8,
                justify="center",
                wraplength=180,
            )
            label.grid(row=row, column=column, sticky="nsew", padx=4, pady=4)
            self.seat_labels[seat] = label
        self.table_center = tk.Label(
            table_box,
            text="Table",
            bg=SURFACE_ALT,
            fg=HEADER,
            relief="solid",
            bd=1,
            padx=8,
            pady=8,
            font=("Segoe UI", 10, "bold"),
        )
        self.table_center.grid(row=1, column=1, sticky="nsew", padx=4, pady=4)

        self.summary = tk.Text(left, width=48, wrap="word", state="disabled", relief="flat", padx=12, pady=10)
        self.summary.configure(bg=SURFACE_ALT, fg=INK, insertbackground=INK, highlightthickness=1, highlightbackground=LINE)
        self.summary.grid(row=1, column=0, sticky="nsew")

        right = ttk.Frame(self.match_frame)
        right.grid(row=2, column=1, sticky="nsew")
        right.columnconfigure(0, weight=1)
        right.rowconfigure(3, weight=1)

        current_box = ttk.LabelFrame(right, text="Current Trick", style="Inner.TLabelframe", padding=6)
        current_box.grid(row=0, column=0, sticky="ew", pady=(0, 6))
        self.current_trick = CardGrid(current_box, self.images, "No cards on table.")
        self.current_trick.pack(fill="x")

        last_box = ttk.LabelFrame(right, text="Last Trick", style="Inner.TLabelframe", padding=6)
        last_box.grid(row=1, column=0, sticky="ew", pady=(0, 6))
        self.last_trick = CardGrid(last_box, self.images, "No completed tricks yet.")
        self.last_trick.pack(fill="x")

        self.actions = ttk.Frame(right)
        self.actions.grid(row=2, column=0, sticky="ew", pady=(0, 6))
        self.actions.columnconfigure(0, weight=1)
        bid_row = ttk.Frame(self.actions)
        bid_row.grid(row=0, column=0, sticky="ew", pady=(0, 3))
        trump_row = ttk.Frame(self.actions)
        trump_row.grid(row=1, column=0, sticky="ew", pady=(0, 3))
        play_row = ttk.Frame(self.actions)
        play_row.grid(row=2, column=0, sticky="ew")
        self.bid_buttons = []
        for label, value in (("Accept / Pass", 0), ("Bid 1", 1), ("Bid 2", 2), ("Bid 3", 3), ("Bid 4", 4)):
            button = ttk.Button(bid_row, text=label, command=lambda v=value: self.bid(v))
            button.pack(side="left", padx=(0, 5), pady=2)
            self.bid_buttons.append(button)
        self.trump_buttons = []
        for suit in ("Hearts", "Diamonds", "Clubs", "Spades"):
            button = ttk.Button(trump_row, text=suit, command=lambda s=suit: self.choose_trump(s))
            button.pack(side="left", padx=(0, 5), pady=2)
            self.trump_buttons.append(button)
        self.play_button = ttk.Button(play_row, text="Play Selected Card", command=self.play_selected_card)
        self.play_button.pack(side="left", padx=(0, 0), pady=2)

        hand_box = ttk.LabelFrame(right, text="Your Cards", style="Inner.TLabelframe", padding=6)
        hand_box.grid(row=3, column=0, sticky="nsew")
        hand_box.columnconfigure(0, weight=1)
        hand_box.rowconfigure(0, weight=1)
        self.hand_canvas = tk.Canvas(
            hand_box,
            bg=SURFACE,
            highlightthickness=1,
            highlightbackground=LINE,
            bd=0,
        )
        self.hand_canvas.grid(row=0, column=0, sticky="nsew")
        self.hand_scrollbar = ttk.Scrollbar(hand_box, orient="vertical", command=self.hand_canvas.yview)
        self.hand_scrollbar.grid(row=0, column=1, sticky="ns")
        self.hand_canvas.configure(yscrollcommand=self.hand_scrollbar.set)
        self.hand_container = ttk.Frame(self.hand_canvas)
        self.hand_window = self.hand_canvas.create_window((0, 0), window=self.hand_container, anchor="nw")
        self.hand_container.bind("<Configure>", self._update_hand_scroll_region)
        self.hand_container.bind("<Enter>", lambda _event: self._bind_hand_mousewheel())
        self.hand_container.bind("<Leave>", lambda _event: self._unbind_hand_mousewheel())
        self.hand_canvas.bind("<Configure>", self._resize_hand_container)
        self.hand_canvas.bind("<Enter>", lambda _event: self._bind_hand_mousewheel())
        self.hand_canvas.bind("<Leave>", lambda _event: self._unbind_hand_mousewheel())
        self.hand = CardGrid(
            self.hand_container,
            self.images,
            "Your cards will appear here.",
            selectable=True,
            on_select=self.select_card,
            on_double=self.play_card_from_double_click,
        )
        self.hand.pack(fill="both", expand=True)

    def _update_hand_scroll_region(self, _event=None) -> None:
        self.hand_canvas.configure(scrollregion=self.hand_canvas.bbox("all"))

    def _resize_hand_container(self, event: tk.Event) -> None:
        width = max(1, event.width - 4)
        self.hand_canvas.itemconfigure(self.hand_window, width=width)
        self.hand.request_render()
        self._update_hand_scroll_region()

    def _bind_hand_mousewheel(self) -> None:
        self.root.bind_all("<MouseWheel>", self._on_hand_mousewheel)

    def _unbind_hand_mousewheel(self) -> None:
        self.root.unbind_all("<MouseWheel>")

    def _on_hand_mousewheel(self, event: tk.Event) -> None:
        self.hand_canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")

    def _authenticate(self, register: bool) -> None:
        username = self.username.get().strip()
        password = self.password.get()
        if not username or not password:
            self.set_message("Enter a username and password.", error=True)
            return

        path = "/api/register" if register else "/api/login"
        data = request_json(path, username=username, password=password, platform="WindowsDesktop")
        self.set_message(data.get("message", ""))
        if data.get("ok"):
            self.current_user = username
            self._save_session(username)
            self.auth_frame.grid_remove()
            self.lobby_frame.grid()
            self.refresh_lobby()

    def _try_restore_session(self) -> None:
        if not SESSION_FILE.exists():
            return
        username = SESSION_FILE.read_text(encoding="utf-8").strip()
        if not username:
            return
        data = request_json("/api/reconnect", username=username, platform="WindowsDesktop")
        if data.get("ok"):
            self.current_user = username
            self.auth_frame.grid_remove()
            self.lobby_frame.grid()
            self.set_message(data.get("message", "Session restored."))
            self.refresh_lobby()
        else:
            self.set_message(data.get("message", "Please log in again."), error=True)

    @staticmethod
    def _save_session(username: str) -> None:
        SESSION_FILE.parent.mkdir(parents=True, exist_ok=True)
        SESSION_FILE.write_text(username, encoding="utf-8")

    def refresh_lobby(self) -> None:
        if not self.current_user:
            return

        data = request_json("/api/lobby", username=self.current_user)
        if not data.get("ok"):
            self.set_message(data.get("message", "User is not logged in."), error=True)
            self._show_logged_out()
            return

        self.waiting_room = data.get("waitingRoom")
        self.rematch_offer = data.get("rematch")
        self.set_message(data.get("notice") or f"Logged in as {self.current_user}.")
        self._render_profile(data.get("profile") or {})
        self._render_online(data)
        self._render_invitations(data.get("invitations", []))
        invite_count = len(data.get("invitations", []))
        if invite_count > self.last_invite_count:
            self._sound("invite")
        self.last_invite_count = invite_count

        if data.get("inMatch"):
            self._show_match()
            self.refresh_match()
        elif data.get("targetSelection"):
            self._show_target()
            self._render_target(data["targetSelection"])
        elif self.rematch_offer:
            self._show_rematch()
            self._render_rematch(self.rematch_offer)
        elif self.waiting_room:
            self._show_lobby()
            self._render_waiting_room(self.waiting_room)
        else:
            self._show_lobby()

    def _render_profile(self, profile: dict) -> None:
        initial = profile.get("avatarInitial") or "?"
        color = profile.get("avatarColor") or HEADER
        self.profile_avatar.configure(text=initial, bg=color)
        recent = profile.get("recentMatches") or []
        lines = [
            f"{profile.get('displayName') or self.current_user}",
            f"Total wins: {profile.get('totalWins', 0)}",
        ]
        if recent:
            lines.append("Recent matches:")
            for match in recent[:3]:
                lines.append(
                    f"- {match.get('completedAt')}: {match.get('winner')} won to {match.get('targetScore')}"
                )
        else:
            lines.append("No completed matches yet.")
        self.profile_text.configure(text="\n".join(lines))

    def refresh_match(self) -> None:
        if not self.current_user or not self.match_frame.winfo_ismapped():
            return

        data = request_json("/api/match", username=self.current_user)
        if not data.get("ok"):
            self.set_message(data.get("message", "Match is not available."), error=True)
            self.refresh_lobby()
            return
        self._render_match(data["state"])

    def _render_online(self, data: dict) -> None:
        selected = [item for item in self.online.selection() if item in self.online.get_children()]
        focus = self.online.focus()
        seen: set[str] = set()
        for player in data.get("onlinePlayers", []):
            username = player.get("username", "")
            seen.add(username)
            display = player.get("displayName") or username
            label = f"{display} (you)" if username == self.current_user else display
            status = "busy" if player.get("inMatch") else "available"
            values = (player.get("platform", ""), status, player.get("totalWins", 0))
            if self.online.exists(username):
                self.online.item(username, text=label, values=values)
            else:
                self.online.insert("", "end", iid=username, text=label, values=values)

        for item in self.online.get_children():
            if item not in seen:
                self.online.delete(item)

        restore = [item for item in selected if self.online.exists(item)]
        if restore:
            self.online.selection_set(restore)
        if focus and self.online.exists(focus):
            self.online.focus(focus)

        in_waiting = bool(self.waiting_room)
        self.invite_button.configure(text="Invite to Room" if in_waiting else "Invite")
        if in_waiting:
            self.game_size_label.pack_forget()
            self.game_size.pack_forget()
        elif not self.game_size.winfo_ismapped():
            self.game_size_label.pack(side="left", padx=(0, 6))
            self.game_size.pack(side="left")

    def _render_invitations(self, invitations: list[dict]) -> None:
        selected = [item for item in self.invitations.selection() if item in self.invitations.get_children()]
        focus = self.invitations.focus()
        seen: set[str] = set()
        for invitation in invitations:
            invite_id = str(invitation.get("id"))
            seen.add(invite_id)
            size = f"{invitation.get('playerCount', 2)} players"
            if self.invitations.exists(invite_id):
                self.invitations.item(invite_id, text=invitation.get("from", ""), values=(size,))
            else:
                self.invitations.insert("", "end", iid=invite_id, text=invitation.get("from", ""), values=(size,))

        for item in self.invitations.get_children():
            if item not in seen:
                self.invitations.delete(item)

        restore = [item for item in selected if self.invitations.exists(item)]
        if restore:
            self.invitations.selection_set(restore)
        if focus and self.invitations.exists(focus):
            self.invitations.focus(focus)

    def _render_waiting_room(self, room: dict) -> None:
        remaining = room.get("playerCount", 0) - len(room.get("players", []))
        if remaining > 0:
            self.waiting_status.configure(text=f"Waiting for {remaining} more player(s).", style="Waiting.TLabel")
        else:
            self.waiting_status.configure(text="Room is full. Waiting for target score selection.", style="Ready.TLabel")

        players = ", ".join(room.get("players", []))
        self.waiting_players.configure(text=f"Room #{room.get('id')} for {room.get('playerCount')} players: {players}")
        lines = [f"{entry.get('from')}: {entry.get('message')}" for entry in room.get("messages", [])]
        self._set_text(self.chat, "\n".join(lines) if lines else "No messages yet.")

    def _render_target(self, selection: dict) -> None:
        chooser = selection.get("chooser")
        player_count = selection.get("playerCount", 2)
        can_choose = chooser == self.current_user
        if can_choose:
            text = f"It's your turn: select the target score for {player_count} players."
        else:
            text = f"Waiting for {chooser} to select the target score for {player_count} players."
        self.target_notice.configure(text=text, style="Ready.TLabel" if can_choose else "Waiting.TLabel")
        for button in self.target_buttons:
            button.configure(state="normal" if can_choose else "disabled")

    def _render_rematch(self, rematch: dict) -> None:
        responded = bool(rematch.get("responded"))
        responses = rematch.get("responses") or {}
        players = rematch.get("players") or []
        accepted = sum(1 for player in players if responses.get(player))
        if responded:
            text = f"Waiting for the other player(s). {accepted}/{len(players)} accepted."
        else:
            text = f"{rematch.get('winner', 'A player')} won the match. Play a rematch?"

        self.rematch_notice.configure(
            text=text,
            style="Waiting.TLabel" if responded else "Ready.TLabel",
        )
        response_text = " | ".join(
            f"{player}: {'yes' if responses.get(player) else 'waiting'}"
            for player in players
        )
        self.rematch_responses.configure(text=response_text)
        state = "disabled" if responded else "normal"
        self.rematch_yes.configure(state=state)
        self.rematch_no.configure(state=state)

    def _render_match(self, state: dict) -> None:
        self.last_match_state = state
        is_my_turn = state.get("currentTurn") == self.current_user
        status = state.get("status", "")
        self._render_match_sounds(state, is_my_turn)

        if status == "Complete":
            self.turn_notice.configure(
                text=f"{state.get('winner') or 'A player'} won the match. Rematch prompt will appear.",
                style="Ready.TLabel",
            )
            self.root.after(1500, self.refresh_lobby)
        else:
            self.turn_notice.configure(
                text=(
                    f"It's your turn: {action_text(status)}."
                    if is_my_turn
                    else f"Waiting for {state.get('currentTurn') or 'the server'}."
                ),
                style="Ready.TLabel" if is_my_turn else "Waiting.TLabel",
            )

        hand = state.get("ownHand", [])
        legal_ids = {int(card_id) for card_id in state.get("legalCardIds", [])}
        can_play = status == "Playing" and is_my_turn
        hand_ids = {card.get("id") for card in hand}
        if self.selected_card_id not in hand_ids or (can_play and self.selected_card_id not in legal_ids):
            self.selected_card_id = None
        rendered_hand = []
        for card in hand:
            copy = dict(card)
            copy["_legal"] = can_play and int(card.get("id", -1)) in legal_ids
            rendered_hand.append(copy)

        self._render_table_seats(state, is_my_turn)
        self._set_text(self.summary, self._match_summary(state))
        self.current_trick.set_cards(self._played_cards(state.get("currentTrick", [])))
        self.last_trick.set_cards(self._played_cards(state.get("lastCompletedTrick", [])))
        self.hand.set_cards(rendered_hand, self.selected_card_id)
        self._update_action_buttons(status, is_my_turn)

    def _render_match_sounds(self, state: dict, is_my_turn: bool) -> None:
        turn_key = f"{state.get('matchId')}:{state.get('status')}:{state.get('currentTurn') or ''}"
        if turn_key != self.last_turn_key and is_my_turn and state.get("status") != "Complete":
            self._sound("turn")
        self.last_turn_key = turn_key

        trick_key = json.dumps(
            [(played.get("player"), (played.get("card") or {}).get("id")) for played in state.get("currentTrick", [])]
        )
        if trick_key != self.last_trick_key and trick_key != "[]":
            self._sound("card")
        self.last_trick_key = trick_key

        winner_key = f"{state.get('matchId')}:{state.get('winner') or ''}"
        if state.get("status") == "Complete" and state.get("winner") and winner_key != self.last_winner_key:
            self._sound("win")
        self.last_winner_key = winner_key

    def _render_table_seats(self, state: dict, is_my_turn: bool) -> None:
        seat_positions = self._seat_positions(len(state.get("players", [])))
        active_player = state.get("currentTurn")
        for seat, label in self.seat_labels.items():
            player = next((item for item in state.get("players", []) if item.get("seat") == seat), None)
            if seat not in seat_positions or player is None:
                label.grid_remove()
                continue
            label.grid()
            is_active = player.get("id") == active_player
            is_you = player.get("id") == self.current_user
            label.configure(
                text=(
                    f"{player.get('name')}{' (you)' if is_you else ''}\n"
                    f"Seat {player.get('seat', 0) + 1} - {player.get('cardsInHand')} cards"
                    f"{team_text(player, state)}"
                    f"{'' if player.get('connected', True) else ' - disconnected'}"
                ),
                bg=SUCCESS_BG if is_active else SURFACE,
                fg=SUCCESS if is_active else INK,
            )
        self.table_center.configure(
            text=f"{'Your move' if is_my_turn else 'Table'}\n{state.get('status', '')}",
            bg=SUCCESS_BG if is_my_turn else SURFACE_ALT,
            fg=SUCCESS if is_my_turn else HEADER,
        )

    @staticmethod
    def _seat_positions(player_count: int) -> set[int]:
        if player_count == 2:
            return {0, 2}
        if player_count == 3:
            return {0, 1, 3}
        return {0, 1, 2, 3}

    def _match_summary(self, state: dict) -> str:
        lines = [
            "Table",
            f"Match: {state.get('matchId')}",
            f"Status: {state.get('status')}",
            f"Turn: {state.get('currentTurn') or 'none'}",
            f"Trump: {state.get('trump') or 'not chosen'}",
            f"Target: {state.get('targetScore')}",
            "",
            "Players",
        ]
        for player in state.get("players", []):
            suffix = " - you" if player.get("id") == self.current_user else ""
            lines.append(
                f"{player.get('name')} ({player.get('id')}) - "
                f"{player.get('cardsInHand')} cards{team_text(player, state)}{suffix}"
            )

        lines.extend(["", "Round Points"])
        round_points = state.get("roundPoints") or {}
        lines.extend([f"{owner_label(owner)}: {score}" for owner, score in round_points.items()] or ["No round points yet."])

        lines.extend(["", "Match Points"])
        scores = state.get("scores") or {}
        lines.extend([f"{owner_label(owner)}: {score}" for owner, score in scores.items()] or ["No match points yet."])

        lines.extend(["", "Bids"])
        bids = state.get("bids") or []
        lines.extend([
            f"{bid.get('player')}: {'accepted / passed' if bid.get('passed') else 'bid ' + str(bid.get('value'))}"
            for bid in bids
        ] or ["No bids yet."])

        lines.extend(["", "Round Results"])
        results = state.get("roundResults") or []
        if results:
            for result in results:
                deltas = ", ".join(
                    f"{owner_label(owner)} {delta:+d}" for owner, delta in (result.get("matchPointDelta") or {}).items()
                )
                lines.append(f"Round {result.get('roundNumber')}: {deltas}")
        else:
            lines.append("No completed rounds yet.")

        if state.get("lastTrickWinner"):
            lines.extend(["", f"Last trick winner: {state['lastTrickWinner']}"])
        return "\n".join(lines)

    def _played_cards(self, played_cards: list[dict]) -> list[dict]:
        cards = []
        for played in played_cards:
            card = dict(played.get("card", {}))
            card["player"] = played.get("player", "")
            cards.append(card)
        return cards

    def _update_action_buttons(self, status: str, is_my_turn: bool) -> None:
        can_bid = status == "Bidding" and is_my_turn
        can_trump = status == "ChoosingTrump" and is_my_turn
        can_play = status == "Playing" and is_my_turn and self._selected_card_is_legal()
        for button in self.bid_buttons:
            button.configure(state="normal" if can_bid else "disabled")
        for button in self.trump_buttons:
            button.configure(state="normal" if can_trump else "disabled")
        self.play_button.configure(state="normal" if can_play else "disabled")

    def _selected_card_is_legal(self) -> bool:
        if self.selected_card_id is None:
            return False
        state = self.last_match_state or {}
        legal_ids = {int(card_id) for card_id in state.get("legalCardIds", [])}
        return self.selected_card_id in legal_ids

    def invite_selected_player(self) -> None:
        selected = self.online.selection()
        if not selected:
            self.set_message("Select an online player first.", error=True)
            return

        to = selected[0]
        if to == self.current_user:
            self.set_message("Select another player.", error=True)
            return

        if self.waiting_room:
            player_count = self.waiting_room.get("playerCount", 3)
            room_id = self.waiting_room.get("id")
            data = request_json(
                "/api/invite",
                **{"from": self.current_user, "to": to, "players": player_count, "room": room_id},
            )
        else:
            player_count = int(self.game_size.get().split()[0])
            data = request_json("/api/invite", **{"from": self.current_user, "to": to, "players": player_count})

        self.set_message(data.get("message", ""), error=not data.get("ok"))
        self.refresh_lobby()

    def respond_invitation(self, accept: bool) -> None:
        selected = self.invitations.selection()
        if not selected:
            self.set_message("Select an invitation first.", error=True)
            return
        data = request_json(
            "/api/respond",
            username=self.current_user,
            invite=selected[0],
            accept=1 if accept else 0,
        )
        self.set_message(data.get("message", ""), error=not data.get("ok"))
        self.refresh_lobby()

    def send_chat(self) -> None:
        message = self.chat_entry.get().strip()
        if not message:
            self.set_message("Enter a chat message first.", error=True)
            return
        data = request_json("/api/chat", username=self.current_user, message=message)
        self.set_message(data.get("message", ""), error=not data.get("ok"))
        if data.get("ok"):
            self.chat_entry.delete(0, "end")
        self.refresh_lobby()

    def respond_rematch(self, accept: bool) -> None:
        data = request_json(
            "/api/rematch",
            username=self.current_user,
            accept=1 if accept else 0,
        )
        self.set_message(data.get("message", ""), error=not data.get("ok"))
        self.refresh_lobby()

    def return_to_lobby(self) -> None:
        data = request_json("/api/cancel", username=self.current_user)
        self.selected_card_id = None
        self.last_match_state = None
        self.set_message(data.get("message", ""), error=not data.get("ok"))
        self.refresh_lobby()

    def select_target(self, score: int) -> None:
        data = request_json("/api/target", username=self.current_user, score=score)
        self.set_message(data.get("message", ""), error=not data.get("ok"))
        self.refresh_lobby()

    def bid(self, value: int) -> None:
        data = request_json("/api/bid", username=self.current_user, value=value)
        self.set_message(data.get("message", ""), error=not data.get("ok"))
        if data.get("state"):
            self._render_match(data["state"])
        if data.get("ok"):
            self.refresh_match()

    def choose_trump(self, suit: str) -> None:
        data = request_json("/api/trump", username=self.current_user, suit=suit)
        self.set_message(data.get("message", ""), error=not data.get("ok"))
        if data.get("state"):
            self._render_match(data["state"])
        if data.get("ok"):
            self.refresh_match()

    def select_card(self, card: dict) -> None:
        if not card.get("_legal", True):
            self.set_message("That card is not legal for the current trick.", error=True)
            return
        self.selected_card_id = card.get("id")
        self.set_message(f"Selected {card.get('label')}.")
        state = self.last_match_state or {}
        if state.get("status") == "Playing" and state.get("currentTurn") == self.current_user:
            self.play_button.configure(state="normal")

    def play_card_from_double_click(self, card: dict) -> None:
        if not card.get("_legal", True):
            self.set_message("That card is not legal for the current trick.", error=True)
            return
        self.selected_card_id = card.get("id")
        self.play_selected_card()

    def play_selected_card(self) -> None:
        if self.selected_card_id is None:
            self.set_message("Select one of your card images first.", error=True)
            return
        if not self._selected_card_is_legal():
            self.set_message("That card is not legal for the current trick.", error=True)
            return
        data = request_json("/api/play", username=self.current_user, card=self.selected_card_id)
        self.set_message(data.get("message", ""), error=not data.get("ok"))
        if data.get("state"):
            self._render_match(data["state"])
        if data.get("ok"):
            self.refresh_match()

    def _show_logged_out(self) -> None:
        self.auth_frame.grid()
        self.lobby_frame.grid_remove()
        self.waiting_frame.grid_remove()
        self.rematch_frame.grid_remove()
        self.target_frame.grid_remove()
        self.match_frame.grid_remove()

    def _show_lobby(self) -> None:
        self.lobby_frame.grid()
        self.match_frame.grid_remove()
        if self.waiting_room:
            self.waiting_frame.grid()
        else:
            self.waiting_frame.grid_remove()
        self.rematch_frame.grid_remove()
        self.target_frame.grid_remove()

    def _show_match(self) -> None:
        self.lobby_frame.grid_remove()
        self.waiting_frame.grid_remove()
        self.rematch_frame.grid_remove()
        self.target_frame.grid_remove()
        self.match_frame.grid()

    def _show_target(self) -> None:
        self.lobby_frame.grid()
        self.waiting_frame.grid_remove()
        self.rematch_frame.grid_remove()
        self.target_frame.grid()
        self.match_frame.grid_remove()

    def _show_rematch(self) -> None:
        self.lobby_frame.grid_remove()
        self.waiting_frame.grid_remove()
        self.rematch_frame.grid()
        self.target_frame.grid_remove()
        self.match_frame.grid_remove()

    def _set_text(self, widget: tk.Text, text: str) -> None:
        widget.configure(state="normal")
        widget.delete("1.0", "end")
        widget.insert("1.0", text)
        widget.configure(state="disabled")
        widget.see("end")

    def set_message(self, text: str, error: bool = False) -> None:
        self.message.configure(text=text or "", foreground="#9f1d1d" if error else "#176b35")
        self.match_message.configure(text=text or "", foreground="#9f1d1d" if error else "#176b35")

    def _sound(self, kind: str) -> None:
        if winsound is None:
            return
        sounds = {
            "turn": winsound.MB_ICONASTERISK,
            "card": winsound.MB_OK,
            "invite": winsound.MB_ICONEXCLAMATION,
            "win": winsound.MB_ICONASTERISK,
        }
        try:
            winsound.MessageBeep(sounds.get(kind, winsound.MB_OK))
        except RuntimeError:
            pass

    def _poll(self) -> None:
        if self.current_user:
            if self.match_frame.winfo_ismapped():
                self.refresh_match()
            else:
                self.refresh_lobby()
        self.root.after(POLL_MS, self._poll)


def main() -> int:
    app = CruceWindowsClient()
    app.run()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
