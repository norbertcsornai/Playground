#include "clients/client_app.hpp"
#include "server/game_server.hpp"
#include "server/user_store.hpp"

#include "test_support.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>

using namespace cruce;
using namespace cruce::clients;
using namespace cruce::server;

namespace {

struct FourClientTable {
  GameServer server;
  MobileClient mobile{server};
  WindowsDesktopClient desktop{server};
  WebClient web{server};
  ClientApp bot{server, ClientPlatform::Bot};
};

FourClientTable start_four_player_table(int target_score = 11) {
  FourClientTable table;
  const auto room = table.server.create_room(4, target_score);
  expect(table.mobile.connect(room, "p1", "Ana").ok, "mobile player joins");
  expect(table.desktop.connect(room, "p2", "Bogdan").ok, "desktop player joins");
  expect(table.web.connect(room, "p3", "Carmen").ok, "web player joins");
  expect(table.bot.connect(room, "p4", "Dan").ok, "bot player joins and starts match");
  return table;
}

void room_starts_mixed_platform_match_with_teams() {
  auto table = start_four_player_table();
  const auto state = table.mobile.state();
  expect(state.has_value(), "state should exist after room fills");
  expect_eq(state->players.size(), std::size_t{4}, "match should have 4 players");
  expect_eq(state->target_score, 11, "room target score should be used");
  expect(state->status == MatchStatus::Bidding, "new match starts in bidding");
  expect_eq(state->own_hand.size(), std::size_t{6}, "4-player hand has 6 cards");
  expect_eq(state->scores.size(), std::size_t{2}, "4-player mode scores two teams");
  expect_eq(state->players[0].owner_id, std::string{"team-0"}, "seat 0 is team 0");
  expect_eq(state->players[2].owner_id, std::string{"team-0"}, "opposite seat is teammate");
  expect_eq(state->players[1].owner_id, std::string{"team-1"}, "seat 1 is team 1");
  expect_eq(state->players[3].owner_id, std::string{"team-1"}, "seat 3 is team 1");
}

void privacy_filter_only_returns_current_players_hand() {
  auto table = start_four_player_table();
  const auto p1_state = table.mobile.state();
  const auto p2_state = table.desktop.state();
  expect(p1_state.has_value(), "p1 state exists");
  expect(p2_state.has_value(), "p2 state exists");
  expect_eq(p1_state->own_hand.size(), std::size_t{6}, "p1 sees own hand");
  expect_eq(p2_state->own_hand.size(), std::size_t{6}, "p2 sees own hand");
  expect(p1_state->own_hand != p2_state->own_hand, "private hands should be different");

  for (const auto& player : p1_state->players) {
    expect_eq(player.cards_in_hand, std::size_t{6}, "public state exposes hand counts only");
  }
}

void server_rejects_out_of_turn_bid() {
  auto table = start_four_player_table();
  const auto rejected = table.mobile.send_bid(1);
  expect(!rejected.ok, "seat 0 should not bid before seat 1");

  const auto state = table.mobile.state();
  expect(state.has_value(), "state should still be available");
  expect_eq(*state->current_turn_player, std::string{"p2"}, "seat after dealer remains current");
}

void clients_can_reach_trump_selection() {
  auto table = start_four_player_table();
  expect(table.desktop.send_bid(1).ok, "p2 bids first");
  expect(table.web.send_bid(0).ok, "p3 passes");
  expect(table.bot.send_bid(0).ok, "p4 passes");
  expect(table.mobile.send_bid(0).ok, "p1 passes");

  auto state = table.desktop.state();
  expect(state.has_value(), "state should exist");
  expect(state->status == MatchStatus::ChoosingTrump, "winner should choose trump");
  expect_eq(*state->current_turn_player, std::string{"p2"}, "p2 won the bid");

  expect(table.desktop.choose_trump(Suit::Spades).ok, "bid winner chooses trump");
  state = table.desktop.state();
  expect(state->status == MatchStatus::Playing, "match should enter play");
  expect_eq(*state->current_turn_player, std::string{"p2"}, "bid winner leads first trick");
  expect(!table.mobile.send_card(state->own_hand.front()).ok, "out-of-turn card play is rejected");
}

void reconnect_updates_connection_state() {
  auto table = start_four_player_table();
  expect(table.desktop.disconnect().ok, "player can disconnect");
  auto state = table.mobile.state();
  expect(state.has_value(), "state should exist");
  expect(!state->players[1].connected, "public state shows disconnected player");

  expect(table.desktop.reconnect().ok, "player can reconnect");
  state = table.mobile.state();
  expect(state->players[1].connected, "public state shows reconnected player");
}

void invalid_room_configuration_is_rejected() {
  GameServer server;
  bool bad_player_count = false;
  bool bad_target_score = false;
  try {
    server.create_room(5, 6);
  } catch (const std::invalid_argument&) {
    bad_player_count = true;
  }
  try {
    server.create_room(4, 15);
  } catch (const std::invalid_argument&) {
    bad_target_score = true;
  }

  expect(bad_player_count, "player count outside 2-4 should throw");
  expect(bad_target_score, "target score outside 6/11/21 should throw");
}

void user_store_seeds_admins_and_registers_users() {
  const std::filesystem::path database_path = "test_users.db";
  std::filesystem::remove(database_path);

  UserStore users{database_path};
  expect(users.initialize().ok, "user database initializes");
  expect(users.authenticate("admin1", "admin1").ok, "admin1 seed account works");
  expect(users.authenticate("admin2", "admin2").ok, "admin2 seed account works");
  expect(!users.authenticate("admin1", "wrong").ok, "wrong password is rejected");
  expect(users.register_user("new_user", "pass123").ok, "new user can register");
  expect(users.authenticate("new_user", "pass123").ok, "new user can log in");
  expect(!users.register_user("new_user", "pass123").ok, "duplicate user is rejected");

  std::filesystem::remove(database_path);
}

}  // namespace

int main() {
  int failures = 0;
  failures += run_test("room_starts_mixed_platform_match_with_teams", room_starts_mixed_platform_match_with_teams);
  failures += run_test("privacy_filter_only_returns_current_players_hand", privacy_filter_only_returns_current_players_hand);
  failures += run_test("server_rejects_out_of_turn_bid", server_rejects_out_of_turn_bid);
  failures += run_test("clients_can_reach_trump_selection", clients_can_reach_trump_selection);
  failures += run_test("reconnect_updates_connection_state", reconnect_updates_connection_state);
  failures += run_test("invalid_room_configuration_is_rejected", invalid_room_configuration_is_rejected);
  failures += run_test("user_store_seeds_admins_and_registers_users", user_store_seeds_admins_and_registers_users);
  return failures == 0 ? 0 : 1;
}
