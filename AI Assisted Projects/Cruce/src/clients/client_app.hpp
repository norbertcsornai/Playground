#pragma once

#include "server/game_server.hpp"

#include <optional>
#include <string>

namespace cruce::clients {

class ClientApp {
 public:
  ClientApp(server::GameServer& server, ClientPlatform platform);
  virtual ~ClientApp() = default;

  ValidationResult connect(
      const std::string& room_code,
      PlayerId player_id,
      std::string display_name);
  ValidationResult reconnect();
  ValidationResult disconnect();

  ValidationResult send_bid(int bid_value);
  ValidationResult choose_trump(Suit suit);
  ValidationResult send_card(const Card& card);
  std::optional<server::GameSnapshot> state() const;

  const PlayerId& player_id() const;
  ClientPlatform platform() const;

 private:
  server::GameServer* server_;
  ClientPlatform platform_;
  PlayerId player_id_;
};

class MobileClient : public ClientApp {
 public:
  explicit MobileClient(server::GameServer& server);
};

class WindowsDesktopClient : public ClientApp {
 public:
  explicit WindowsDesktopClient(server::GameServer& server);
};

class WebClient : public ClientApp {
 public:
  explicit WebClient(server::GameServer& server);
};

}  // namespace cruce::clients
