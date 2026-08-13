#include "server/privacy_filter.hpp"

namespace cruce::server {

GameSnapshot PrivacyFilter::private_state_for(
    const Match& match,
    const PlayerId& player_id) {
  GameSnapshot snapshot;
  snapshot.match_id = match.id();
  snapshot.status = match.status();
  snapshot.target_score = match.target_score();
  snapshot.dealer_seat = match.dealer_seat();
  snapshot.current_turn_player = match.current_turn_player();
  snapshot.scores = match.score_board().scores();
  snapshot.round_results = match.round_results();
  snapshot.winner = match.winner();

  for (const auto& player : match.players()) {
    snapshot.players.push_back(PlayerSnapshot{
        player.id(),
        player.display_name(),
        player.platform(),
        player.seat_index(),
        player.connected(),
        player.hand().size(),
        match.owner_for_player(player.id()),
    });

    if (player.id() == player_id) {
      snapshot.own_hand = player.hand();
    }
  }

  for (const auto& card : match.legal_cards_for(player_id)) {
    snapshot.legal_card_ids.push_back(card.id());
  }

  if (const auto* round = match.active_round()) {
    snapshot.trump_suit = round->trump_suit();
    snapshot.bids = round->bids();
    snapshot.round_points = round->card_points();
    snapshot.current_trick = round->current_trick().played_cards();
    if (!round->completed_tricks().empty() && round->trump_suit().has_value()) {
      const auto& last_trick = round->completed_tricks().back();
      snapshot.last_completed_trick = last_trick.played_cards();
      snapshot.last_trick_winner = last_trick.winner(*round->trump_suit()).player_id;
    }
  }

  return snapshot;
}

}  // namespace cruce::server
