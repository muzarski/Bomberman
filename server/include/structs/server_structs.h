#ifndef SERVER_STRUCTS_H
#define SERVER_STRUCTS_H

#include "../../../common/include/common_structs.h"
#include <variant>
#include <random>
#include <optional>
#include <chrono>

struct Args {
    uint16_t bomb_timer{};
    uint8_t players_count{};
    uint16_t players_count_u16{};
    uint64_t turn_duration{};
    uint16_t explosion_radius{};
    uint16_t initial_blocks{};
    uint16_t game_length{};
    std::string server_name;
    uint16_t port{};
    uint32_t seed{static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count())};
    uint16_t size_x{};
    uint16_t size_y{};
};

class BombPlacedEvent {
private:
    uint32_t bomb_id{};
    Position bomb_position{};

public:
    BombPlacedEvent();
    BombPlacedEvent(uint32_t bomb_id_, Position bomb_position_);
    void serialize(Buffer &buffer) const;
};

class BombExplodedEvent {
private:
    uint32_t bomb_id{};
    std::set<uint8_t> robots_destroyed;
    std::set<Position> blocks_destroyed;

public:
    BombExplodedEvent();
    explicit BombExplodedEvent(uint32_t bomb_id_);
    void robot_destroyed(uint8_t robot_id);
    void block_destroyed(Position block_position);
    void serialize(Buffer &buffer) const;
};

class PlayerMovedEvent {
private:
    uint8_t player_id{};
    Position new_position{};

public:
    PlayerMovedEvent();
    PlayerMovedEvent(uint8_t player_id_, Position new_position_);
    void serialize(Buffer &buffer) const;
};

class BlockPlacedEvent {
private:
    Position block_position{};

public:
    BlockPlacedEvent();
    explicit BlockPlacedEvent(Position block_position_);
    void serialize(Buffer &buffer) const;
};

enum State { LOBBY, GAME };
enum SendMessage { NOTHING, ACCEPTED_PLAYER, GAME_STARTED, TURN, GAME_ENDED };

using Event = std::variant<BombPlacedEvent, BombExplodedEvent, PlayerMovedEvent, BlockPlacedEvent>;

class GameInfo {
public:
    explicit GameInfo(Args &&args);
    [[nodiscard]] std::optional<Position> move_if_valid(Position from, uint8_t direction) const;
    [[nodiscard]] Buffer produce_hello_message() const;
    [[nodiscard]] Buffer produce_accepted_player_message() const;
    Buffer produce_game_started_message();
    Buffer produce_turn_message();
    Buffer produce_game_ended_message();
    void compute_explosions();
    void bomb_row_explosion(Position starting_position, std::set<Position> &blocks_exploded, BombExplodedEvent &event, uint8_t direction);

    uint16_t size_x;
    uint16_t size_y;
    uint16_t game_length;
    uint16_t explosion_radius;
    uint16_t bomb_timer;
    uint8_t players_count;
    uint64_t turn_duration;
    std::string server_name;
    uint16_t initial_blocks;

    std::map<std::string, Player> players;
    std::map<uint8_t, Player> player_id_to_player_info;
    std::vector<Position> players_position;
    std::map<uint8_t, uint32_t> scores;
    std::vector<bool> died_this_turn;
    std::map<Position, std::set<uint8_t>> players_position_reversed;
    std::minstd_rand random;

    uint8_t players_joined;
    State state;
    SendMessage what_message;

    Player added_player;

    uint16_t current_turn;
    uint32_t current_bomb_id;
    std::map<uint32_t, Bomb> bombs;
    std::set<Position> blocks;
    std::vector<Event> events;
    std::vector<Buffer> previous_turn_messages;
    std::vector<Buffer> previous_accepted_player_messages;
};

#endif // SERVER_STRUCTS_H
