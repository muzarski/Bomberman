#ifndef CLIENT_STRUCTS_H
#define CLIENT_STRUCTS_H

#include "../../../common/include/common_structs.h"

enum Action { SEND_LOBBY, SEND_GAME, DO_NOT_SEND };

struct ClientInfo {
    ClientInfo();

    bool received_hello;
    bool sent_join;
    Action action;

    std::string server_name;
    uint8_t players_count{};
    uint16_t size_x{};
    uint16_t size_y{};
    uint16_t game_length{};
    uint16_t explosion_radius{};
    uint16_t bomb_timer{};
    std::map<uint8_t, Player> players;

    uint16_t turn{};
    std::map<uint8_t, Position> player_positions;
    std::set<Position> blocks;
    std::map<uint32_t, Bomb> bombs;
    std::set<Position> explosions;
    std::map<uint8_t, uint32_t> scores;

    std::map<uint8_t, bool> already_dead;
    std::set<Position> blocks_copy;

    [[nodiscard]] Buffer produce_lobby_message() const;
    [[nodiscard]] Buffer produce_game_message() const;
    void print_lobby_message() const;
    void print_game_message() const;
};

#endif // CLIENT_STRUCTS_H
