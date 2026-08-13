#include "clients/client_app.hpp"
#include "core/types.hpp"
#include "server/game_server.hpp"

#include <iostream>

int main() {
  cruce::server::GameServer server;
  const auto room_code = server.create_room(4, 6);

  cruce::clients::MobileClient mobile(server);
  cruce::clients::WindowsDesktopClient desktop(server);
  cruce::clients::WebClient web(server);
  cruce::clients::ClientApp bot(server, cruce::ClientPlatform::Bot);

  mobile.connect(room_code, "p1", "Ana");
  desktop.connect(room_code, "p2", "Bogdan");
  web.connect(room_code, "p3", "Carmen");
  bot.connect(room_code, "p4", "Dan");

  const auto state = mobile.state();
  if (!state.has_value()) {
    std::cerr << "Unable to create demo match.\n";
    return 1;
  }

  std::cout << "Cruce demo server started an in-memory match\n";
  std::cout << "Room: " << room_code << "\n";
  std::cout << "Match: " << state->match_id << "\n";
  std::cout << "Players: " << state->players.size() << "\n";
  std::cout << "Target score: " << state->target_score << "\n";
  std::cout << "Status: " << cruce::to_string(state->status) << "\n";
  std::cout << "Ana can see " << state->own_hand.size() << " cards\n";

  if (state->current_turn_player.has_value()) {
    std::cout << "Current turn: " << *state->current_turn_player << "\n";
  }

  return 0;
}
