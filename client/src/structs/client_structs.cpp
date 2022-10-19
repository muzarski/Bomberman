#include "../../include/structs/client_structs.h"
#include <iostream>

ClientInfo::ClientInfo() :
        received_hello(false),
        sent_join(false) {}

[[maybe_unused]] void ClientInfo::print_lobby_message() const {
    std::cout << "[LOBBY MESSAGE]\n";
    std::cout << "message_id: " << 0 << std::endl;
    std::cout << "server_name " << server_name << std::endl;
    std::cout << "players_count " << +players_count << std::endl;
    std::cout << "size_x " << size_x << std::endl;
    std::cout << "size_y " << size_y << std::endl;
    std::cout << "game_length " << game_length << std::endl;
    std::cout << "explosion_radius " << explosion_radius << std::endl;
    std::cout << "bomb_timer " << bomb_timer << std::endl;
    std::cout << "players " << std::endl;
    for (auto &entry : players) {
        std::cout << "  player_id " << +entry.first << ", ";
        std::cout << "player_name " << entry.second.name << ", ";
        std::cout << "player_address " << entry.second.address << std::endl;
    }
}

[[maybe_unused]] void ClientInfo::print_game_message() const {
    std::cout << "[GAME MESSAGE]\n";
    std::cout << "message_id: " << 1 << std::endl;
    std::cout << "server_name: " << server_name << std::endl;
    std::cout << "size_x " << size_x << std::endl;
    std::cout << "size_y " << size_y << std::endl;
    std::cout << "game_length " << game_length << std::endl;
    std::cout << "turn " << turn << std::endl;
    std::cout << "players " << std::endl;
    for (auto &entry : players) {
        std::cout << "  player_id " << +entry.first << ", ";
        std::cout << "player_name " << entry.second.name << ", ";
        std::cout << "player_address " << entry.second.address << std::endl;
    }
    std::cout << "player_positions\n";
    for (auto &entry : player_positions) {
        std::cout << "  player_id " << +entry.first << ", ";
        std::cout << "x " << entry.second.x << ", ";
        std::cout << "y " << entry.second.y << std::endl;
    }
    std::cout << "blocks:\n";
    for (auto &entry : blocks) {
        std::cout << "  x " << entry.x << ", y " << entry.y << std::endl;
    }
    std::cout << "bombs\n";
    for (auto &entry : bombs) {
        std::cout << "  bomb_id " << entry.first << ", ";
        std::cout << "x " << entry.second.bomb_position.x << ", ";
        std::cout << "y " << entry.second.bomb_position.y << ", ";
        std::cout << "timer " << entry.second.bomb_timer << std::endl;
    }

    std::cout << "explosions\n";
    for (auto &entry : explosions) {
        std::cout << "  x " << entry.x << ", y " << entry.y << std::endl;
    }

    std::cout << "scores\n";
    for(auto &entry : scores) {
        std::cout << "  player_id " << +entry.first << ", ";
        std::cout << "  player_score "  << entry.second << std::endl;
    }
}

Buffer ClientInfo::produce_lobby_message() const {
    Buffer res(128);
    res.write_primitive<uint8_t>(0); // message_id
    res.serialize_string(server_name);
    res.write_primitive<uint8_t>(players_count);
    res.write_primitive<uint16_t>(htons(size_x));
    res.write_primitive<uint16_t>(htons(size_y));
    res.write_primitive<uint16_t>(htons(game_length));
    res.write_primitive<uint16_t>(htons(explosion_radius));
    res.write_primitive<uint16_t>(htons(bomb_timer));
    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(players.size())));
    for (auto &entry : players) {
        res.write_primitive<uint8_t>(entry.first);
        res.serialize_string(entry.second.name);
        res.serialize_string(entry.second.address);
    }

    return res;
}

Buffer ClientInfo::produce_game_message() const {
    Buffer res(128);
    res.write_primitive<uint8_t>(1); // message_id
    res.serialize_string(server_name);
    res.write_primitive<uint16_t>(htons(size_x));
    res.write_primitive<uint16_t>(htons(size_y));
    res.write_primitive<uint16_t>(htons(game_length));
    res.write_primitive<uint16_t>(htons(turn));

    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(players.size())));
    for (auto &entry : players) {
        res.write_primitive<uint8_t>(entry.first);
        res.serialize_string(entry.second.name);
        res.serialize_string(entry.second.address);
    }

    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(player_positions.size())));
    for (auto &entry : player_positions) {
        res.write_primitive<uint8_t>(entry.first);
        res.write_primitive<uint16_t>(htons(entry.second.x));
        res.write_primitive<uint16_t>(htons(entry.second.y));
    }

    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(blocks.size())));
    for (auto &pos : blocks) {
        res.write_primitive<uint16_t>(htons(pos.x));
        res.write_primitive<uint16_t>(htons(pos.y));
    }

    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(bombs.size())));
    for (auto &bomb : bombs) {
        res.write_primitive<uint16_t>(htons(bomb.second.bomb_position.x));
        res.write_primitive<uint16_t>(htons(bomb.second.bomb_position.y));
        res.write_primitive<uint16_t>(htons(bomb.second.bomb_timer));
    }

    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(explosions.size())));
    for (auto &expl : explosions) {
        res.write_primitive<uint16_t>(htons(expl.x));
        res.write_primitive<uint16_t>(htons(expl.y));
    }

    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(scores.size())));
    for (auto &score : scores) {
        res.write_primitive<uint8_t>(score.first);
        res.write_primitive<uint32_t>(htonl(score.second));
    }

    return res;
}
