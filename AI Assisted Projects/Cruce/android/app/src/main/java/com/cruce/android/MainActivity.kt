package com.cruce.android

import android.app.Activity
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Color
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.media.AudioManager
import android.media.ToneGenerator
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.Editable
import android.text.InputType
import android.text.TextWatcher
import android.view.Gravity
import android.view.View
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.GridLayout
import android.widget.HorizontalScrollView
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.Spinner
import android.widget.TextView
import android.widget.Toast
import org.json.JSONArray
import org.json.JSONObject
import java.net.HttpURLConnection
import java.net.URL
import java.net.URLEncoder
import java.util.Collections
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.Executors

private const val DEFAULT_SERVER_URL = "http://192.168.2.17:8080"
private const val POLL_MS = 2000L
private const val PREF_SERVER_BASE = "serverBase"
private const val PREF_LAST_USERNAME = "lastUsername"

private val APP_BG = Color.rgb(244, 247, 245)
private val SURFACE = Color.rgb(255, 255, 255)
private val SURFACE_ALT = Color.rgb(248, 250, 249)
private val HEADER = Color.rgb(21, 60, 53)
private val INK = Color.rgb(27, 43, 40)
private val MUTED = Color.rgb(95, 113, 108)
private val LINE = Color.rgb(215, 222, 217)
private val ACCENT = Color.rgb(168, 79, 43)
private val GOLD = Color.rgb(223, 182, 76)
private val SUCCESS = Color.rgb(20, 83, 49)
private val SUCCESS_BG = Color.rgb(226, 244, 233)
private val WAITING = Color.rgb(112, 75, 18)
private val WAITING_BG = Color.rgb(255, 240, 196)
private val DANGER = Color.rgb(159, 29, 29)
private val DANGER_BG = Color.rgb(253, 232, 232)
private val DISABLED_BG = Color.rgb(207, 217, 212)

class MainActivity : Activity() {
    private enum class Screen {
        Login,
        Lobby,
        Target,
        Rematch,
        Match
    }

    private lateinit var scroll: ScrollView
    private lateinit var content: LinearLayout
    private val mainHandler = Handler(Looper.getMainLooper())
    private val apiExecutor = Executors.newSingleThreadExecutor()
    private val imageExecutor = Executors.newFixedThreadPool(3)
    private val imageCache = ConcurrentHashMap<String, Bitmap>()
    private val imageLoading = Collections.synchronizedSet(mutableSetOf<String>())
    private val toneGenerator = ToneGenerator(AudioManager.STREAM_NOTIFICATION, 45)

    private var serverBase = DEFAULT_SERVER_URL
    private var currentUser: String? = null
    private var restoringSession = false
    private var selectedPlayerCount = 2
    private var selectedCardId: Int? = null
    private var lastMatchState: JSONObject? = null
    private var lastRenderedMatchFingerprint: String? = null
    private var lastTurnKey = ""
    private var lastTrickKey = ""
    private var lastWinnerKey = ""
    private var lastInviteCount = 0
    private var currentScreen = Screen.Login
    private var chatDraft = ""

    private val pollRunnable = object : Runnable {
        override fun run() {
            if (currentUser != null) {
                refreshLobby()
                mainHandler.postDelayed(this, POLL_MS)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val prefs = getPreferences(MODE_PRIVATE)
        serverBase = prefs.getString(PREF_SERVER_BASE, DEFAULT_SERVER_URL) ?: DEFAULT_SERVER_URL

        scroll = ScrollView(this)
        content = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(14), dp(14), dp(14), dp(24))
            setBackgroundColor(APP_BG)
        }
        scroll.addView(content)
        setContentView(scroll)

        val savedUser = prefs.getString(PREF_LAST_USERNAME, "")?.trim().orEmpty()
        if (savedUser.isNotBlank()) {
            currentUser = savedUser
            restoringSession = true
            renderBase(Screen.Lobby, "Lobby", "Restoring session...")
            reconnectSavedUser(savedUser)
        } else {
            renderLogin()
        }
    }

    override fun onResume() {
        super.onResume()
        if (currentUser != null && !restoringSession) {
            refreshLobby()
            startPolling()
        }
    }

    override fun onPause() {
        mainHandler.removeCallbacks(pollRunnable)
        super.onPause()
    }

    override fun onDestroy() {
        mainHandler.removeCallbacksAndMessages(null)
        apiExecutor.shutdownNow()
        imageExecutor.shutdownNow()
        toneGenerator.release()
        super.onDestroy()
    }

    private fun renderLogin(message: String? = null) {
        currentUser = null
        restoringSession = false
        mainHandler.removeCallbacks(pollRunnable)
        selectedCardId = null
        lastMatchState = null
        lastRenderedMatchFingerprint = null
        currentScreen = Screen.Login
        content.removeAllViews()
        addTitle("Cruce")
        addText("Enter the server running on your PC, then log in or create an account.", 15)
        message?.let { addNotice(it, error = true) }

        val serverInput = addInput("Server URL", serverBase, false)
        val usernameInput = addInput("Username", getPreferences(MODE_PRIVATE).getString(PREF_LAST_USERNAME, "") ?: "", false)
        val passwordInput = addInput("Password", "", true)

        val buttons = addRow(content)
        addButton(buttons, "Login") {
            authenticate(serverInput.text.toString(), usernameInput.text.toString(), passwordInput.text.toString(), false)
        }
        addButton(buttons, "Register") {
            authenticate(serverInput.text.toString(), usernameInput.text.toString(), passwordInput.text.toString(), true)
        }
    }

    private fun authenticate(server: String, username: String, password: String, register: Boolean) {
        val cleanServer = normalizeServerUrl(server)
        val cleanUser = username.trim()
        if (cleanServer.isBlank() || cleanUser.isBlank() || password.isBlank()) {
            toast("Enter server URL, username, and password.")
            return
        }

        serverBase = cleanServer
        getPreferences(MODE_PRIVATE).edit().putString(PREF_SERVER_BASE, serverBase).apply()
        val path = if (register) "/api/register" else "/api/login"
        api(
            path,
            mapOf("username" to cleanUser, "password" to password, "platform" to "Mobile")
        ) { data ->
            toast(data.optString("message", ""))
            if (data.optBoolean("ok")) {
                currentUser = cleanUser
                getPreferences(MODE_PRIVATE).edit()
                    .putString(PREF_SERVER_BASE, serverBase)
                    .putString(PREF_LAST_USERNAME, cleanUser)
                    .apply()
                refreshLobby()
                startPolling()
            }
        }
    }

    private fun reconnectSavedUser(username: String) {
        api("/api/reconnect", mapOf("username" to username, "platform" to "Mobile")) { data ->
            restoringSession = false
            if (data.optBoolean("ok")) {
                toast(data.optString("message", "Session restored."))
                refreshLobby()
                startPolling()
            } else {
                renderLogin(data.optString("message", "Please log in again."))
            }
        }
    }

    private fun startPolling() {
        mainHandler.removeCallbacks(pollRunnable)
        mainHandler.postDelayed(pollRunnable, POLL_MS)
    }

    private fun refreshLobby() {
        val user = currentUser ?: return
        api("/api/lobby", mapOf("username" to user)) { data ->
            if (!data.optBoolean("ok")) {
                renderLogin(data.optString("message", "User is not logged in."))
                return@api
            }

            when {
                data.optBoolean("inMatch") -> refreshMatch()
                data.nullableObject("targetSelection") != null ->
                    renderTarget(data, data.nullableObject("targetSelection")!!)
                data.nullableObject("rematch") != null ->
                    renderRematch(data, data.nullableObject("rematch")!!)
                else -> renderLobby(data, data.nullableObject("waitingRoom"))
            }
        }
    }

    private fun refreshMatch() {
        val user = currentUser ?: return
        api("/api/match", mapOf("username" to user)) { data ->
            if (!data.optBoolean("ok")) {
                refreshLobby()
                return@api
            }
            renderMatch(data.getJSONObject("state"))
        }
    }

    private fun renderLobby(data: JSONObject, waitingRoom: JSONObject?) {
        renderBase(Screen.Lobby, "Lobby", data.optString("notice", ""))
        renderProfilePanel(data.nullableObject("profile"))

        val onlinePanel = addPanel("Online Players")
        if (waitingRoom == null) {
            val chooserRow = addRow(onlinePanel)
            addTextTo(chooserRow, "Invite to:", 15, bold = true)
            val spinner = Spinner(this)
            val options = listOf("2 players", "3 players", "4 players")
            spinner.adapter = ArrayAdapter(this, android.R.layout.simple_spinner_dropdown_item, options)
            spinner.setSelection((selectedPlayerCount - 2).coerceIn(0, 2))
            spinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
                override fun onItemSelected(parent: AdapterView<*>, view: View?, position: Int, id: Long) {
                    selectedPlayerCount = position + 2
                }

                override fun onNothingSelected(parent: AdapterView<*>) = Unit
            }
            chooserRow.addView(spinner)
        } else {
            addNoticeTo(onlinePanel, "Invite available players to room #${waitingRoom.optInt("id")}.", false)
        }

        val onlinePlayers = data.array("onlinePlayers")
        if (onlinePlayers.length() == 0) {
            addTextTo(onlinePanel, "No online players.", 15)
        }
        for (player in onlinePlayers.objects()) {
            val name = player.optString("username")
            val busy = player.optBoolean("inMatch")
            val isMe = name == currentUser
            val row = addRow(onlinePanel)
            addAvatarTo(row, player.optString("avatarInitial", "?"), player.optString("avatarColor", "#153c35"))
            val status = if (busy) "busy" else "available"
            addTextTo(
                row,
                "${player.optString("displayName", name)}${if (isMe) " (you)" else ""}\n" +
                    "${player.optString("platform")} - $status - ${player.optInt("totalWins")} wins",
                15,
                weight = 1f
            )
            addButton(row, if (waitingRoom == null) "Invite" else "Invite to Room", enabled = !isMe && !busy) {
                invitePlayer(name, waitingRoom)
            }
        }

        val invitesPanel = addPanel("Invitations")
        val invitations = data.array("invitations")
        if (invitations.length() > lastInviteCount) {
            playTone("invite")
        }
        lastInviteCount = invitations.length()
        if (invitations.length() == 0) {
            addTextTo(invitesPanel, "No pending invitations.", 15)
        }
        for (invitation in invitations.objects()) {
            val row = addRow(invitesPanel)
            addTextTo(
                row,
                "${invitation.optString("from")} invited you to a ${invitation.optInt("playerCount", 2)}-player game.",
                15,
                weight = 1f
            )
            addButton(row, "Accept") { respondInvitation(invitation.optInt("id"), true) }
            addButton(row, "Decline") { respondInvitation(invitation.optInt("id"), false) }
        }

        if (waitingRoom != null) {
            renderWaitingRoomPanel(waitingRoom)
        }
    }

    private fun renderProfilePanel(profile: JSONObject?) {
        val panel = addPanel("Your Profile")
        if (profile == null) {
            addTextTo(panel, "No profile loaded.", 15)
            return
        }
        val row = addRow(panel)
        addAvatarTo(row, profile.optString("avatarInitial", "?"), profile.optString("avatarColor", "#153c35"))
        addTextTo(
            row,
            "${profile.optString("displayName", currentUser ?: "")}\nTotal wins: ${profile.optInt("totalWins")}",
            15,
            bold = true,
            weight = 1f
        )
        val recent = profile.array("recentMatches").objects()
        if (recent.isEmpty()) {
            addTextTo(panel, "No completed matches yet.", 14)
        } else {
            addTextTo(panel, "Recent matches", 15, bold = true)
            for (match in recent.take(3)) {
                addTextTo(
                    panel,
                    "${match.optString("completedAt")}: ${match.optString("winner")} won to ${match.optInt("targetScore")}",
                    14
                )
            }
        }
    }

    private fun renderWaitingRoomPanel(room: JSONObject) {
        val panel = addPanel("Waiting Room")
        val players = room.array("players").strings()
        val remaining = room.optInt("playerCount") - players.size
        addNoticeTo(
            panel,
            if (remaining > 0) "Waiting for $remaining more player(s)." else "Room is full. Waiting for target selection.",
            error = false
        )
        addTextTo(panel, "Room #${room.optInt("id")} for ${room.optInt("playerCount")} players", 15, bold = true)
        addTextTo(panel, players.joinToString(", "), 15)

        addTextTo(panel, "Chat", 16, bold = true)
        val messages = room.array("messages").objects()
        if (messages.isEmpty()) {
            addTextTo(panel, "No messages yet.", 15)
        } else {
            for (message in messages.takeLast(10)) {
                addTextTo(panel, "${message.optString("from")}: ${message.optString("message")}", 15)
            }
        }

        val chatInput = addInputTo(panel, "Message", chatDraft, false)
        chatInput.addTextChangedListener(object : TextWatcher {
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) = Unit
            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
                chatDraft = s?.toString() ?: ""
            }

            override fun afterTextChanged(s: Editable?) = Unit
        })
        chatInput.setOnFocusChangeListener { _, hasFocus ->
            if (!hasFocus) {
                chatDraft = chatInput.text.toString()
            }
        }
        val row = addRow(panel)
        addButton(row, "Send") {
            chatDraft = chatInput.text.toString()
            sendChat()
        }
        addButton(row, "Return to Lobby") { returnToLobby() }
    }

    private fun renderTarget(lobbyData: JSONObject, selection: JSONObject) {
        val chooser = selection.optString("chooser")
        val canChoose = chooser == currentUser
        renderBase(Screen.Target, "Target Score", lobbyData.optString("notice", ""))
        val panel = addPanel("Choose Match Length")
        addNoticeTo(
            panel,
            if (canChoose) "It's your turn: choose 6, 11, or 21 points."
            else "Waiting for $chooser to choose the target score.",
            error = false
        )
        addTextTo(panel, "Players: ${selection.array("players").strings().joinToString(", ")}", 15)
        val row = addRow(panel)
        for (score in listOf(6, 11, 21)) {
            addButton(row, "$score", enabled = canChoose) { selectTarget(score) }
        }
        addButton(panel, "Return to Lobby") { returnToLobby() }
    }

    private fun renderRematch(lobbyData: JSONObject, rematch: JSONObject) {
        renderBase(Screen.Rematch, "Rematch", lobbyData.optString("notice", ""))
        val panel = addPanel("Play Again")
        val responded = rematch.optBoolean("responded")
        val responses = rematch.nullableObject("responses") ?: JSONObject()
        val players = rematch.array("players").strings()
        val accepted = players.count { responses.optBoolean(it) }
        addNoticeTo(
            panel,
            if (responded) "Waiting for the other player(s). $accepted/${players.size} accepted."
            else "${rematch.optString("winner", "A player")} won the match. Play a rematch?",
            error = false
        )
        addTextTo(panel, players.joinToString(" | ") { "$it: ${if (responses.optBoolean(it)) "yes" else "waiting"}" }, 15)
        val row = addRow(panel)
        addButton(row, "Yes", enabled = !responded) { respondRematch(true) }
        addButton(row, "No", enabled = !responded) { respondRematch(false) }
        addButton(panel, "Return to Lobby") { returnToLobby() }
    }

    private fun renderMatch(state: JSONObject) {
        val fingerprint = state.toString()
        if (currentScreen == Screen.Match && fingerprint == lastRenderedMatchFingerprint) {
            return
        }

        val preserveScroll = currentScreen == Screen.Match
        lastRenderedMatchFingerprint = fingerprint
        lastMatchState = state
        val user = currentUser ?: return
        val status = state.optString("status")
        val isMyTurn = state.optString("currentTurn") == user
        val canPlayCards = status == "Playing" && isMyTurn
        val legalIds = state.array("legalCardIds").ints().toSet()
        val hand = state.array("ownHand").objects()
        if (hand.none { it.optInt("id", -1) == selectedCardId } ||
            (canPlayCards && selectedCardId != null && selectedCardId !in legalIds)
        ) {
            selectedCardId = null
        }
        renderMatchSounds(state, isMyTurn)

        renderBase(Screen.Match, "Match", "", preserveScroll = preserveScroll)
        val table = addPanel("Table")
        val winner = state.optString("winner", "")
        val notice = when {
            status == "Complete" -> "${if (winner.isBlank()) "A player" else winner} won the match. Waiting for rematch prompt."
            isMyTurn -> "It's your turn: ${actionText(status)}."
            else -> "Waiting for ${state.optString("currentTurn", "the server")}."
        }
        addNoticeTo(table, notice, error = false)
        addButton(table, "Return to Lobby") { returnToLobby() }
        addTextTo(
            table,
            "Status: ${state.optString("status")} | Turn: ${state.optString("currentTurn", "none")} | " +
                "Trump: ${state.optString("trump", "not chosen")} | Target: ${state.optInt("targetScore")}",
            15
        )
        addSeatMap(table, state, isMyTurn)

        val actions = addPanel("Actions")
        val canBid = status == "Bidding" && isMyTurn
        val bidRow1 = addRow(actions)
        addButton(bidRow1, "Accept / Pass", enabled = canBid) { bid(0) }
        addButton(bidRow1, "Bid 1", enabled = canBid) { bid(1) }
        addButton(bidRow1, "Bid 2", enabled = canBid) { bid(2) }
        val bidRow2 = addRow(actions)
        addButton(bidRow2, "Bid 3", enabled = canBid) { bid(3) }
        addButton(bidRow2, "Bid 4", enabled = canBid) { bid(4) }

        val canTrump = status == "ChoosingTrump" && isMyTurn
        val trumpRow1 = addRow(actions)
        addButton(trumpRow1, "Hearts", enabled = canTrump) { chooseTrump("Hearts") }
        addButton(trumpRow1, "Diamonds", enabled = canTrump) { chooseTrump("Diamonds") }
        val trumpRow2 = addRow(actions)
        addButton(trumpRow2, "Clubs", enabled = canTrump) { chooseTrump("Clubs") }
        addButton(trumpRow2, "Spades", enabled = canTrump) { chooseTrump("Spades") }

        addCardGrid("Your Cards", JSONArray().apply { hand.forEach { put(it) } }, playable = canPlayCards, legalIds = legalIds)
        addCardStrip("Current Trick", state.array("currentTrick").playedCards())
        addCardStrip("Last Completed Trick", state.array("lastCompletedTrick").playedCards())

        val details = addPanel("Match Details")
        addMatchSummary(details, state)

        if (status == "Complete") {
            mainHandler.postDelayed({ refreshLobby() }, 1200)
        }
    }

    private fun renderMatchSounds(state: JSONObject, isMyTurn: Boolean) {
        val turnKey = "${state.optString("matchId")}:${state.optString("status")}:${state.optString("currentTurn", "")}"
        if (turnKey != lastTurnKey && isMyTurn && state.optString("status") != "Complete") {
            playTone("turn")
        }
        lastTurnKey = turnKey

        val trickKey = state.array("currentTrick").objects()
            .joinToString("|") { "${it.optString("player")}:${it.nullableObject("card")?.optInt("id", -1)}" }
        if (trickKey != lastTrickKey && trickKey.isNotBlank()) {
            playTone("card")
        }
        lastTrickKey = trickKey

        val winnerKey = "${state.optString("matchId")}:${state.optString("winner", "")}"
        if (state.optString("status") == "Complete" && state.optString("winner").isNotBlank() && winnerKey != lastWinnerKey) {
            playTone("win")
        }
        lastWinnerKey = winnerKey
    }

    private fun addSeatMap(parent: LinearLayout, state: JSONObject, isMyTurn: Boolean) {
        val grid = GridLayout(this).apply {
            columnCount = 3
            rowCount = 3
            useDefaultMargins = false
        }
        parent.addView(grid, fullWidthParams(top = 4, bottom = 6))

        val playerCount = state.array("players").length()
        val active = state.optString("currentTurn")
        val positions = seatPositions(playerCount)
        val playersBySeat = state.array("players").objects().associateBy { it.optInt("seat") }
        for (seat in positions.keys.sorted()) {
            val player = playersBySeat[seat] ?: continue
            val isActive = player.optString("id") == active
            val isYou = player.optString("id") == currentUser
            val cell = LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                gravity = Gravity.CENTER
                setPadding(dp(8), dp(8), dp(8), dp(8))
                background = rounded(if (isActive) SUCCESS_BG else SURFACE, if (isActive) SUCCESS else LINE, 1)
                if (isActive) {
                    animate().scaleX(1.02f).scaleY(1.02f).setDuration(160).withEndAction {
                        animate().scaleX(1f).scaleY(1f).setDuration(160).start()
                    }.start()
                }
            }
            addTextTo(
                cell,
                "${player.optString("name")}${if (isYou) " (you)" else ""}\n" +
                    "Seat ${player.optInt("seat") + 1} - ${player.optInt("cardsInHand")} cards${teamText(player, state)}",
                13,
                bold = isActive
            )
            val position = positions[seat] ?: Pair(1, 1)
            grid.addView(cell, GridLayout.LayoutParams(GridLayout.spec(position.first), GridLayout.spec(position.second)).apply {
                width = 0
                height = GridLayout.LayoutParams.WRAP_CONTENT
                columnSpec = GridLayout.spec(position.second, 1f)
                setMargins(dp(3), dp(3), dp(3), dp(3))
            })
        }
        val center = TextView(this).apply {
            text = if (isMyTurn) "Your move" else state.optString("status")
            gravity = Gravity.CENTER
            textSize = 14f
            typeface = Typeface.DEFAULT_BOLD
            setTextColor(if (isMyTurn) SUCCESS else HEADER)
            setPadding(dp(8), dp(8), dp(8), dp(8))
            background = rounded(if (isMyTurn) SUCCESS_BG else SURFACE_ALT, LINE, 1)
        }
        grid.addView(center, GridLayout.LayoutParams(GridLayout.spec(1), GridLayout.spec(1)).apply {
            width = 0
            columnSpec = GridLayout.spec(1, 1f)
            setMargins(dp(3), dp(3), dp(3), dp(3))
        })
    }

    private fun seatPositions(playerCount: Int): Map<Int, Pair<Int, Int>> {
        return when (playerCount) {
            2 -> mapOf(0 to Pair(2, 1), 2 to Pair(0, 1))
            3 -> mapOf(0 to Pair(2, 1), 1 to Pair(1, 0), 3 to Pair(1, 2))
            else -> mapOf(0 to Pair(2, 1), 1 to Pair(1, 0), 2 to Pair(0, 1), 3 to Pair(1, 2))
        }
    }

    private fun addMatchSummary(parent: LinearLayout, state: JSONObject) {
        addTextTo(
            parent,
            "Status: ${state.optString("status")}\n" +
                "Turn: ${state.optString("currentTurn", "none")}\n" +
                "Trump: ${state.optString("trump", "not chosen")}\n" +
                "Target: ${state.optInt("targetScore")}",
            15
        )

        addTextTo(parent, "Players", 16, bold = true)
        for (player in state.array("players").objects()) {
            val you = if (player.optString("id") == currentUser) " - you" else ""
            addTextTo(
                parent,
                "${player.optString("name")} (${player.optString("id")}) - " +
                    "${player.optInt("cardsInHand")} cards${teamText(player, state)}$you",
                14
            )
        }

        addTextTo(parent, "Round Points", 16, bold = true)
        addTextTo(parent, keyValues(state.nullableObject("roundPoints"), labelOwners = true), 14)
        addTextTo(parent, "Match Points", 16, bold = true)
        addTextTo(parent, keyValues(state.nullableObject("scores"), labelOwners = true), 14)

        val bids = state.array("bids").objects()
        addTextTo(parent, "Bids", 16, bold = true)
        if (bids.isEmpty()) {
            addTextTo(parent, "No bids yet.", 14)
        } else {
            for (bid in bids) {
                val value = if (bid.optBoolean("passed")) "accepted / passed" else "bid ${bid.optInt("value")}"
                addTextTo(parent, "${bid.optString("player")}: $value", 14)
            }
        }

        val lastWinner = state.optString("lastTrickWinner", "")
        if (lastWinner.isNotBlank() && lastWinner != "null") {
            addTextTo(parent, "Last trick winner: $lastWinner", 14)
        }
    }

    private fun addCardStrip(title: String, cards: JSONArray, selectable: Boolean = false) {
        val panel = addPanel(title)
        if (cards.length() == 0) {
            addTextTo(panel, "No cards to show.", 15)
            return
        }

        val horizontal = HorizontalScrollView(this)
        val row = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.TOP
        }
        horizontal.addView(row)
        panel.addView(horizontal, fullWidthParams())

        for (card in cards.objects()) {
            row.addView(cardView(card, selectable), cardLinearParams())
        }
    }

    private fun addCardGrid(title: String, cards: JSONArray, playable: Boolean = false, legalIds: Set<Int> = emptySet()) {
        val panel = addPanel(title)
        if (cards.length() == 0) {
            addTextTo(panel, "No cards to show.", 15)
            return
        }
        if (playable && legalIds.size < cards.length()) {
            addTextTo(panel, "Only highlighted cards are legal for this trick.", 14)
        }

        val grid = GridLayout(this).apply {
            columnCount = cardGridColumns()
            alignmentMode = GridLayout.ALIGN_BOUNDS
            useDefaultMargins = false
        }
        panel.addView(grid, fullWidthParams())

        for (card in cards.objects()) {
            grid.addView(cardView(card, selectable = playable, legal = playable && legalIds.contains(card.optInt("id", -1))), cardGridParams())
        }
    }

    private fun cardView(card: JSONObject, selectable: Boolean, legal: Boolean = true): LinearLayout {
        val cardId = card.optInt("id", -1)
        val selected = selectable && legal && selectedCardId == cardId
        val box = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL
            setPadding(dp(6), dp(6), dp(6), dp(6))
            background = rounded(
                when {
                    selected -> SUCCESS_BG
                    !legal && selectable -> Color.rgb(238, 242, 239)
                    else -> Color.rgb(255, 253, 249)
                },
                if (selected || (selectable && legal)) SUCCESS else LINE,
                if (selected || (selectable && legal)) 2 else 1
            )
            elevation = dp(1).toFloat()
            alpha = if (!legal && selectable) 0.55f else 1f
        }

        val image = ImageView(this).apply {
            adjustViewBounds = true
            scaleType = ImageView.ScaleType.FIT_CENTER
            setBackgroundColor(Color.rgb(255, 253, 249))
        }
        box.addView(image, LinearLayout.LayoutParams(dp(88), dp(128)))
        loadCardImage(card.optString("image"), image)

        val title = buildString {
            if (card.optString("player").isNotBlank()) {
                append(card.optString("player"))
                append(": ")
            }
            append(card.optString("label", "Card"))
        }
        box.addView(TextView(this).apply {
            text = title
            textSize = 12f
            gravity = Gravity.CENTER
            setTextColor(INK)
        }, fullWidthParams())

        if (selectable) {
            box.setOnClickListener {
                if (!legal) {
                    toast("That card is not legal for the current trick.")
                    return@setOnClickListener
                }
                selectedCardId = cardId
                box.animate().translationY(-dp(5).toFloat()).setDuration(90).withEndAction {
                    box.animate().translationY(0f).setDuration(90).start()
                }.start()
                playSelectedCard()
            }
            box.setOnLongClickListener {
                if (!legal) {
                    toast("That card is not legal for the current trick.")
                    return@setOnLongClickListener true
                }
                selectedCardId = cardId
                playSelectedCard()
                true
            }
        }
        return box
    }
    private fun invitePlayer(to: String, waitingRoom: JSONObject?) {
        val user = currentUser ?: return
        val params = if (waitingRoom != null) {
            mapOf(
                "from" to user,
                "to" to to,
                "players" to waitingRoom.optInt("playerCount", 3).toString(),
                "room" to waitingRoom.optInt("id").toString()
            )
        } else {
            mapOf("from" to user, "to" to to, "players" to selectedPlayerCount.toString())
        }
        api("/api/invite", params) { data ->
            toast(data.optString("message", ""))
            refreshLobby()
        }
    }

    private fun respondInvitation(inviteId: Int, accept: Boolean) {
        api(
            "/api/respond",
            mapOf("username" to (currentUser ?: ""), "invite" to inviteId.toString(), "accept" to if (accept) "1" else "0")
        ) { data ->
            toast(data.optString("message", ""))
            refreshLobby()
        }
    }

    private fun sendChat() {
        val message = chatDraft.trim()
        if (message.isBlank()) {
            toast("Enter a chat message first.")
            return
        }
        api("/api/chat", mapOf("username" to (currentUser ?: ""), "message" to message)) { data ->
            toast(data.optString("message", ""))
            if (data.optBoolean("ok")) {
                chatDraft = ""
            }
            refreshLobby()
        }
    }

    private fun respondRematch(accept: Boolean) {
        api("/api/rematch", mapOf("username" to (currentUser ?: ""), "accept" to if (accept) "1" else "0")) { data ->
            toast(data.optString("message", ""))
            refreshLobby()
        }
    }

    private fun returnToLobby() {
        api("/api/cancel", mapOf("username" to (currentUser ?: ""))) { data ->
            selectedCardId = null
            lastMatchState = null
            toast(data.optString("message", ""))
            refreshLobby()
        }
    }

    private fun selectTarget(score: Int) {
        api("/api/target", mapOf("username" to (currentUser ?: ""), "score" to score.toString())) { data ->
            toast(data.optString("message", ""))
            refreshLobby()
        }
    }

    private fun bid(value: Int) {
        api("/api/bid", mapOf("username" to (currentUser ?: ""), "value" to value.toString())) { data ->
            toast(data.optString("message", ""))
            data.nullableObject("state")?.let { renderMatch(it) } ?: refreshMatch()
        }
    }

    private fun chooseTrump(suit: String) {
        api("/api/trump", mapOf("username" to (currentUser ?: ""), "suit" to suit)) { data ->
            toast(data.optString("message", ""))
            data.nullableObject("state")?.let { renderMatch(it) } ?: refreshMatch()
        }
    }

    private fun playSelectedCard() {
        val card = selectedCardId
        if (card == null) {
            toast("Select one of your cards first.")
            return
        }
        val legalIds = lastMatchState?.array("legalCardIds")?.ints()?.toSet().orEmpty()
        if (card !in legalIds) {
            toast("That card is not legal for the current trick.")
            return
        }
        api("/api/play", mapOf("username" to (currentUser ?: ""), "card" to card.toString())) { data ->
            toast(data.optString("message", ""))
            if (data.optBoolean("ok")) {
                selectedCardId = null
            }
            data.nullableObject("state")?.let { renderMatch(it) } ?: refreshMatch()
        }
    }

    private fun api(path: String, params: Map<String, String> = emptyMap(), callback: (JSONObject) -> Unit) {
        val base = serverBase.trimEnd('/')
        val query = params.entries.joinToString("&") {
            "${encode(it.key)}=${encode(it.value)}"
        }
        val url = if (query.isBlank()) "$base$path" else "$base$path?$query"

        apiExecutor.execute {
            val result = try {
                val connection = URL(url).openConnection() as HttpURLConnection
                connection.requestMethod = "GET"
                connection.connectTimeout = 4000
                connection.readTimeout = 4000
                connection.setRequestProperty("Connection", "close")
                val stream = if (connection.responseCode in 200..399) {
                    connection.inputStream
                } else {
                    connection.errorStream ?: connection.inputStream
                }
                val body = stream.bufferedReader().use { it.readText() }
                JSONObject(body)
            } catch (ex: Exception) {
                JSONObject()
                    .put("ok", false)
                    .put("message", "Cannot reach Cruce server: ${ex.message}")
            }
            mainHandler.post {
                if (!isDestroyed) {
                    callback(result)
                }
            }
        }
    }

    private fun loadCardImage(path: String, imageView: ImageView) {
        if (path.isBlank()) {
            imageView.setImageDrawable(null)
            return
        }
        val url = if (path.startsWith("http")) path else "${serverBase.trimEnd('/')}$path"
        imageView.tag = url
        imageCache[url]?.let {
            imageView.setImageBitmap(it)
            return
        }
        imageView.setImageDrawable(null)
        if (!imageLoading.add(url)) {
            return
        }

        imageExecutor.execute {
            val bitmap = try {
                URL(url).openStream().use { stream ->
                    BitmapFactory.decodeStream(stream)
                }
            } catch (_: Exception) {
                null
            }
            if (bitmap != null) {
                imageCache[url] = bitmap
            }
            imageLoading.remove(url)
            mainHandler.post {
                if (!isDestroyed && imageView.tag == url && bitmap != null) {
                    imageView.setImageBitmap(bitmap)
                }
            }
        }
    }

    private fun renderBase(screenName: Screen, title: String, notice: String?, preserveScroll: Boolean = false) {
        val previousScrollY = if (preserveScroll && currentScreen == screenName) scroll.scrollY else 0
        currentScreen = screenName
        if (screenName != Screen.Match) {
            lastRenderedMatchFingerprint = null
        }
        content.removeAllViews()
        addTitle(title)
        val user = currentUser
        if (user != null) {
            addText("Logged in as $user\nServer: $serverBase", 14)
        }
        if (!notice.isNullOrBlank()) {
            addNotice(notice, error = false)
        }
        if (previousScrollY > 0) {
            scroll.post {
                val maxScrollY = (content.height - scroll.height).coerceAtLeast(0)
                scroll.scrollTo(0, previousScrollY.coerceAtMost(maxScrollY))
            }
        }
    }
    private fun addTitle(text: String) {
        val header = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(14), dp(12), dp(14), dp(10))
            background = rounded(HEADER, HEADER, 0)
            elevation = dp(2).toFloat()
        }
        header.addView(TextView(this).apply {
            this.text = text
            textSize = 26f
            typeface = Typeface.DEFAULT_BOLD
            setTextColor(Color.WHITE)
        }, fullWidthParams(bottom = 9))
        header.addView(View(this).apply {
            setBackgroundColor(GOLD)
        }, LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, dp(4)))
        content.addView(header, fullWidthParams(bottom = 12))
    }

    private fun addText(text: String, size: Int) {
        addTextTo(content, text, size)
    }

    private fun addTextTo(parent: LinearLayout, text: String, size: Int, bold: Boolean = false, weight: Float = 0f) {
        val width = when {
            weight > 0f -> 0
            parent.orientation == LinearLayout.HORIZONTAL -> LinearLayout.LayoutParams.WRAP_CONTENT
            else -> LinearLayout.LayoutParams.MATCH_PARENT
        }
        parent.addView(TextView(this).apply {
            this.text = text.ifBlank { "None." }
            textSize = size.toFloat()
            setTextColor(INK)
            if (bold) {
                typeface = Typeface.DEFAULT_BOLD
            }
        }, LinearLayout.LayoutParams(width, LinearLayout.LayoutParams.WRAP_CONTENT, weight).apply {
            setMargins(0, dp(3), 0, dp(3))
        })
    }

    private fun addNotice(text: String, error: Boolean) {
        addNoticeTo(content, text, error)
    }

    private fun addNoticeTo(parent: LinearLayout, text: String, error: Boolean) {
        parent.addView(TextView(this).apply {
            this.text = text
            textSize = 15f
            typeface = Typeface.DEFAULT_BOLD
            setPadding(dp(12), dp(9), dp(12), dp(9))
            setTextColor(if (error) DANGER else SUCCESS)
            background = rounded(
                if (error) DANGER_BG else SUCCESS_BG,
                if (error) DANGER else SUCCESS,
                1
            )
        }, fullWidthParams(bottom = 10))
    }

    private fun addPanel(title: String): LinearLayout {
        val panel = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(13), dp(11), dp(13), dp(13))
            background = rounded(SURFACE, LINE, 1)
            elevation = dp(2).toFloat()
        }
        panel.addView(TextView(this).apply {
            text = title
            textSize = 18f
            typeface = Typeface.DEFAULT_BOLD
            setTextColor(HEADER)
        }, fullWidthParams(bottom = 8))
        content.addView(panel, fullWidthParams(bottom = 12))
        return panel
    }

    private fun addRow(parent: LinearLayout): LinearLayout {
        val row = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
        }
        parent.addView(row, fullWidthParams(bottom = 6))
        return row
    }

    private fun addButton(parent: LinearLayout, text: String, enabled: Boolean = true, action: () -> Unit): Button {
        val button = Button(this).apply {
            this.text = text
            isEnabled = enabled
            isAllCaps = false
            typeface = Typeface.DEFAULT_BOLD
            minHeight = dp(44)
            setPadding(dp(12), 0, dp(12), 0)
            setTextColor(if (enabled) Color.WHITE else MUTED)
            background = rounded(if (enabled) ACCENT else DISABLED_BG, if (enabled) ACCENT else DISABLED_BG, 1)
            setOnClickListener { action() }
        }
        parent.addView(button, LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT).apply {
            setMargins(0, dp(3), dp(6), dp(3))
        })
        return button
    }

    private fun addAvatarTo(parent: LinearLayout, initial: String, color: String) {
        parent.addView(TextView(this).apply {
            text = initial.ifBlank { "?" }.take(1).uppercase()
            gravity = Gravity.CENTER
            textSize = 17f
            typeface = Typeface.DEFAULT_BOLD
            setTextColor(Color.WHITE)
            background = rounded(parseColor(color, HEADER), parseColor(color, HEADER), 0)
        }, LinearLayout.LayoutParams(dp(42), dp(42)).apply {
            setMargins(0, dp(3), dp(9), dp(3))
        })
    }

    private fun playTone(kind: String) {
        val tone = when (kind) {
            "turn" -> ToneGenerator.TONE_PROP_ACK
            "invite" -> ToneGenerator.TONE_PROP_BEEP2
            "win" -> ToneGenerator.TONE_PROP_ACK
            else -> ToneGenerator.TONE_PROP_BEEP
        }
        try {
            toneGenerator.startTone(tone, 120)
        } catch (_: RuntimeException) {
        }
    }

    private fun addInput(label: String, value: String, password: Boolean): EditText {
        return addInputTo(content, label, value, password)
    }

    private fun addInputTo(parent: LinearLayout, label: String, value: String, password: Boolean): EditText {
        parent.addView(TextView(this).apply {
            text = label
            textSize = 14f
            typeface = Typeface.DEFAULT_BOLD
            setTextColor(HEADER)
        }, fullWidthParams(top = 6))
        val input = EditText(this).apply {
            setText(value)
            setSingleLine(true)
            hint = label
            setTextColor(INK)
            setHintTextColor(MUTED)
            setPadding(dp(10), 0, dp(10), 0)
            minHeight = dp(48)
            background = rounded(SURFACE, LINE, 1)
            inputType = if (password) {
                InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_PASSWORD
            } else {
                InputType.TYPE_CLASS_TEXT
            }
        }
        parent.addView(input, fullWidthParams(bottom = 6))
        return input
    }

    private fun fullWidthParams(top: Int = 0, bottom: Int = 0): LinearLayout.LayoutParams {
        return LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        ).apply {
            setMargins(0, dp(top), 0, dp(bottom))
        }
    }

    private fun cardLinearParams(): LinearLayout.LayoutParams {
        return LinearLayout.LayoutParams(dp(112), LinearLayout.LayoutParams.WRAP_CONTENT).apply {
            setMargins(0, dp(4), dp(8), dp(4))
        }
    }

    private fun cardGridParams(): GridLayout.LayoutParams {
        return GridLayout.LayoutParams().apply {
            width = dp(112)
            height = LinearLayout.LayoutParams.WRAP_CONTENT
            setMargins(0, dp(4), dp(8), dp(8))
        }
    }

    private fun cardGridColumns(): Int {
        val availableWidth = resources.displayMetrics.widthPixels - dp(52)
        return (availableWidth / dp(112)).coerceIn(2, 4)
    }

    private fun rounded(fill: Int, stroke: Int, strokeDp: Int): GradientDrawable {
        return GradientDrawable().apply {
            setColor(fill)
            cornerRadius = dp(8).toFloat()
            setStroke(dp(strokeDp), stroke)
        }
    }

    private fun parseColor(value: String, fallback: Int): Int {
        return try {
            Color.parseColor(value)
        } catch (_: IllegalArgumentException) {
            fallback
        }
    }

    private fun keyValues(json: JSONObject?, labelOwners: Boolean = false): String {
        if (json == null || json.length() == 0) {
            return "None yet."
        }
        val lines = mutableListOf<String>()
        val keys = json.keys()
        while (keys.hasNext()) {
            val key = keys.next()
            val label = if (labelOwners) ownerLabel(key) else key
            lines.add("$label: ${json.opt(key)}")
        }
        return lines.joinToString("\n")
    }

    private fun ownerLabel(owner: String): String {
        val match = Regex("^team-(\\d+)$").matchEntire(owner)
        return if (match != null) {
            "Team ${match.groupValues[1].toInt() + 1}"
        } else {
            owner
        }
    }

    private fun teamText(player: JSONObject, state: JSONObject): String {
        return if (state.array("players").length() == 4 && player.optString("team").isNotBlank()) {
            " - ${player.optString("team")}"
        } else {
            ""
        }
    }

    private fun actionText(status: String): String {
        return when (status) {
            "Bidding" -> "select a bid or accept/pass"
            "ChoosingTrump" -> "choose trump"
            "Playing" -> "play a legal card"
            else -> "wait for the next round"
        }
    }

    private fun normalizeServerUrl(input: String): String {
        val trimmed = input.trim()
        if (trimmed.isBlank()) {
            return ""
        }
        val withScheme = if (trimmed.startsWith("http://") || trimmed.startsWith("https://")) {
            trimmed
        } else {
            "http://$trimmed"
        }
        return withScheme.trimEnd('/')
    }

    private fun encode(value: String): String {
        return URLEncoder.encode(value, "UTF-8")
    }

    private fun toast(message: String) {
        if (message.isNotBlank()) {
            Toast.makeText(this, message, Toast.LENGTH_SHORT).show()
        }
    }

    private fun dp(value: Int): Int {
        return (value * resources.displayMetrics.density).toInt()
    }
}

private fun JSONObject.array(name: String): JSONArray = optJSONArray(name) ?: JSONArray()

private fun JSONObject.nullableObject(name: String): JSONObject? {
    return if (isNull(name)) null else optJSONObject(name)
}

private fun JSONArray.objects(): List<JSONObject> {
    val result = mutableListOf<JSONObject>()
    for (index in 0 until length()) {
        optJSONObject(index)?.let { result.add(it) }
    }
    return result
}

private fun JSONArray.strings(): List<String> {
    val result = mutableListOf<String>()
    for (index in 0 until length()) {
        result.add(optString(index))
    }
    return result
}

private fun JSONArray.ints(): List<Int> {
    val result = mutableListOf<Int>()
    for (index in 0 until length()) {
        result.add(optInt(index))
    }
    return result
}

private fun JSONArray.playedCards(): JSONArray {
    val result = JSONArray()
    for (played in objects()) {
        val card = played.nullableObject("card") ?: JSONObject()
        val copy = JSONObject(card.toString())
        copy.put("player", played.optString("player"))
        result.put(copy)
    }
    return result
}
