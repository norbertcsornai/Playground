#pragma once

#include "core/deck.hpp"
#include "core/rules_engine.hpp"
#include "core/types.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace cruce {

class Player {
 public:
  Player() = default;
  Player(PlayerId id, std::string display_name, ClientPlatform platform, int seat_index);

  const PlayerId& id() const;
  const std::string& display_name() const;
  ClientPlatform platform() const;
  int seat_index() const;
  bool connected() const;

  void set_connected(bool connected);
  void clear_hand();
  void receive_card(const Card& card);
  void receive_cards(const std::vector<Card>& cards);
  bool has_card(const Card& card) const;
  bool remove_card(const Card& card);
  const std::vector<Card>& hand() const;

 private:
  PlayerId id_;
  std::string display_name_;
  ClientPlatform platform_ = ClientPlatform::Bot;
  int seat_index_ = 0;
  bool connected_ = true;
  std::vector<Card> hand_;
};

class Team {
 public:
  Team() = default;
  Team(OwnerId owner_id, std::vector<PlayerId> player_ids);

  const OwnerId& owner_id() const;
  const std::vector<PlayerId>& player_ids() const;
  bool contains(const PlayerId& player_id) const;

 private:
  OwnerId owner_id_;
  std::vector<PlayerId> player_ids_;
};

struct Announcement {
  PlayerId player_id;
  Suit suit = Suit::Hearts;
  int points = 0;
};

struct RoundResult {
  int round_number = 0;
  PlayerId bid_winner;
  OwnerId bidder_owner;
  int bid_value = 0;
  Suit trump_suit = Suit::Hearts;
  std::map<OwnerId, int> card_points;
  std::map<OwnerId, int> match_point_delta;
  bool bid_succeeded = false;
  std::vector<Announcement> announcements;
};

class Trick {
 public:
  Trick() = default;
  explicit Trick(int leader_seat);

  int leader_seat() const;
  const std::vector<PlayedCard>& played_cards() const;
  void add_card(const PlayedCard& played_card);
  PlayedCard winner(Suit trump_suit) const;
  int points() const;
  bool complete(int player_count) const;

 private:
  int leader_seat_ = 0;
  std::vector<PlayedCard> played_cards_;
};

class Round {
 public:
  Round() = default;
  Round(int round_number, int player_count, int dealer_seat);

  int round_number() const;
  int player_count() const;
  int dealer_seat() const;
  int leader_seat() const;
  const std::optional<Suit>& trump_suit() const;
  const std::optional<Bid>& winning_bid() const;
  const std::vector<Bid>& bids() const;
  const Trick& current_trick() const;
  const std::vector<Trick>& completed_tricks() const;
  const std::map<OwnerId, int>& card_points() const;
  const std::vector<Announcement>& announcements() const;
  const Deck& deck() const;
  bool draw_pile_empty() const;
  bool bidding_complete() const;
  bool first_lead_must_be_trump() const;

 private:
  friend class Match;

  int round_number_ = 0;
  int player_count_ = 0;
  int dealer_seat_ = 0;
  int leader_seat_ = 0;
  Deck deck_;
  std::optional<Suit> trump_suit_;
  std::optional<Bid> winning_bid_;
  std::vector<Bid> bids_;
  std::vector<int> bidding_order_;
  std::size_t next_bid_index_ = 0;
  Trick current_trick_;
  std::vector<Trick> completed_tricks_;
  std::map<OwnerId, int> card_points_;
  std::vector<Announcement> announcements_;
  std::set<Suit> announced_suits_;
  bool first_lead_must_be_trump_ = false;
};

class ScoreBoard {
 public:
  ScoreBoard() = default;
  explicit ScoreBoard(const std::vector<OwnerId>& owners);

  void add_round_delta(const std::map<OwnerId, int>& delta_by_owner);
  void add_round_score(const OwnerId& owner_id, int points);
  void apply_failed_bid(const OwnerId& owner_id, int bid_value);
  std::optional<OwnerId> leader() const;
  std::optional<OwnerId> winner(int target_score) const;
  const std::map<OwnerId, int>& scores() const;

 private:
  std::map<OwnerId, int> scores_;
};

class Match {
 public:
  Match() = default;
  Match(std::string match_id, std::vector<Player> players, int target_score);

  ValidationResult start_round(std::uint32_t shuffle_seed);
  ValidationResult submit_bid(const PlayerId& player_id, int bid_value);
  ValidationResult choose_trump(const PlayerId& player_id, Suit suit);
  ValidationResult play_card(const PlayerId& player_id, const Card& card);

  const std::string& id() const;
  int target_score() const;
  int dealer_seat() const;
  MatchStatus status() const;
  const std::vector<Player>& players() const;
  const std::vector<Team>& teams() const;
  const ScoreBoard& score_board() const;
  const std::vector<RoundResult>& round_results() const;
  const std::optional<OwnerId>& winner() const;
  const Round* active_round() const;

  std::optional<PlayerId> current_turn_player() const;
  std::vector<Card> legal_cards_for(const PlayerId& player_id) const;
  const Player* find_player(const PlayerId& player_id) const;
  Player* find_player(const PlayerId& player_id);
  OwnerId owner_for_player(const PlayerId& player_id) const;

 private:
  Player& player_at_seat(int seat_index);
  const Player& player_at_seat(int seat_index) const;
  int seat_for_player(const PlayerId& player_id) const;
  int next_seat_after(int seat_index) const;
  int cards_per_player() const;
  bool all_hands_empty() const;
  void initialize_owners();
  void initialize_round_points();
  void complete_current_trick();
  void finish_round();
  void maybe_score_announcement(Player& player, const Card& card);

  std::string match_id_;
  int target_score_ = 6;
  int dealer_seat_ = 0;
  MatchStatus status_ = MatchStatus::WaitingForPlayers;
  std::vector<Player> players_;
  std::vector<Team> teams_;
  ScoreBoard score_board_;
  std::optional<Round> active_round_;
  std::vector<RoundResult> round_results_;
  std::optional<OwnerId> winner_;
  std::uint32_t next_shuffle_seed_ = 1000;
};

}  // namespace cruce
