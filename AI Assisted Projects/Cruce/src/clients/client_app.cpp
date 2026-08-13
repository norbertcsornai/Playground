#include "clients/client_app.hpp"

#include <utility>

namespace cruce::clients {

ClientApp::ClientApp(server::GameServer& server, ClientPlatform platform)
    : server_(&server), platform_(platform) {}

ValidationResult ClientApp::connect(
    const std::string& room_code,
    PlayerId player_id,
    std::string display_name) {
  player_id_ = std::move(player_id);
  return server_->join_room(
      room_code,
      server::JoinRequest{player_id_, std::move(display_name), platform_});
}

ValidationResult ClientApp::reconnect() {
  return server_->reconnect_player(player_id_);
}

ValidationResult ClientApp::disconnect() {
  return server_->disconnect_player(player_id_);
}

ValidationResult ClientApp::send_bid(int bid_value) {
  return server_->submit_bid(player_id_, bid_value);
}

ValidationResult ClientApp::choose_trump(Suit suit) {
  return server_->choose_trump(player_id_, suit);
}

ValidationResult ClientApp::send_card(const Card& card) {
  return server_->submit_card_play(player_id_, card);
}

std::optional<server::GameSnapshot> ClientApp::state() const {
  return server_->snapshot_for(player_id_);
}

const PlayerId& ClientApp::player_id() const {
  return player_id_;
}

ClientPlatform ClientApp::platform() const {
  return platform_;
}

MobileClient::MobileClient(server::GameServer& server)
    : ClientApp(server, ClientPlatform::Mobile) {}

WindowsDesktopClient::WindowsDesktopClient(server::GameServer& server)
    : ClientApp(server, ClientPlatform::WindowsDesktop) {}

WebClient::WebClient(server::GameServer& server)
    : ClientApp(server, ClientPlatform::Web) {}

}  // namespace cruce::clients
