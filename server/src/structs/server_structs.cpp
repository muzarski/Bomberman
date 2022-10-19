#include "../../include/structs/server_structs.h"
#include <iostream>

BombPlacedEvent::BombPlacedEvent() = default;
BombPlacedEvent::BombPlacedEvent(uint32_t bomb_id_, Position bomb_position_) :
        bomb_id(bomb_id_),
        bomb_position(bomb_position_) {};

void BombPlacedEvent::serialize(Buffer &buffer) const {
    buffer.write_primitive<uint8_t>(0);
    buffer.write_primitive<uint32_t>(htonl(bomb_id));
    buffer.write_primitive<uint16_t>(htons(bomb_position.x));
    buffer.write_primitive<uint16_t>(htons(bomb_position.y));
}


BombExplodedEvent::BombExplodedEvent() = default;
BombExplodedEvent::BombExplodedEvent(uint32_t bomb_id_) :
        bomb_id(bomb_id_) {};

void BombExplodedEvent::robot_destroyed(uint8_t robot_id) {
    robots_destroyed.insert(robot_id);
}
void BombExplodedEvent::block_destroyed(Position block_position) {
    blocks_destroyed.insert(block_position);
}

void BombExplodedEvent::serialize(Buffer &buffer) const {
    buffer.write_primitive<uint8_t>(1);
    buffer.write_primitive<uint32_t>(htonl(bomb_id));
    buffer.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(robots_destroyed.size())));
    for (auto &robot_id : robots_destroyed) {
        buffer.write_primitive<uint8_t>(robot_id);
    }

    buffer.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(blocks_destroyed.size())));
    for (auto &block_position : blocks_destroyed) {
        buffer.write_primitive<uint16_t>(htons(block_position.x));
        buffer.write_primitive<uint16_t>(htons(block_position.y));
    }
}


PlayerMovedEvent::PlayerMovedEvent() = default;
PlayerMovedEvent::PlayerMovedEvent(uint8_t player_id_, Position new_position_) :
        player_id(player_id_),
        new_position(new_position_) {};

void PlayerMovedEvent::serialize(Buffer &buffer) const {
    buffer.write_primitive<uint8_t>(2);
    buffer.write_primitive<uint8_t>(player_id);
    buffer.write_primitive<uint16_t>(htons(new_position.x));
    buffer.write_primitive<uint16_t>(htons(new_position.y));
}


BlockPlacedEvent::BlockPlacedEvent() = default;
BlockPlacedEvent::BlockPlacedEvent(Position block_position_) :
        block_position(block_position_) {};

void BlockPlacedEvent::serialize(Buffer &buffer) const {
    buffer.write_primitive<uint8_t>(3);
    buffer.write_primitive<uint16_t>(htons(block_position.x));
    buffer.write_primitive<uint16_t>(htons(block_position.y));
}


GameInfo::GameInfo(Args &&args) :
        size_x(args.size_x),
        size_y(args.size_y),
        game_length(args.game_length),
        explosion_radius(args.explosion_radius),
        bomb_timer(args.bomb_timer),
        players_count(args.players_count),
        turn_duration(args.turn_duration),
        server_name(args.server_name),
        initial_blocks(args.initial_blocks),
        random(args.seed),
        players_joined(0),
        state(LOBBY),
        what_message(NOTHING),
        added_player(),
        current_turn(0) {};

std::optional<Position> GameInfo::move_if_valid(Position from, uint8_t direction) const {
    Position new_position = from;
    switch (direction) {
        case 0: // UP
            new_position.go_up();
            if (from.y < size_y - 1 && !blocks.contains(new_position))
                return new_position;
            return std::nullopt;
        case 1: // RIGHT
            new_position.go_right();
            if (from.x < size_x - 1 && !blocks.contains(new_position))
                return new_position;
            return std::nullopt;
        case 2: // DOWN
            new_position.go_down();
            if (from.y > 0 && !blocks.contains(new_position))
                return new_position;
            return std::nullopt;
        case 3: // LEFT
            new_position.go_left();
            if (from.x > 0 && !blocks.contains(new_position))
                return new_position;
            return std::nullopt;
        default:
            return std::nullopt;
    }
}

Buffer GameInfo::produce_hello_message() const {
    Buffer res(128);
    res.write_primitive<uint8_t>(0);
    res.serialize_string(server_name);
    res.write_primitive<uint8_t>(players_count);
    res.write_primitive<uint16_t>(htons(size_x));
    res.write_primitive<uint16_t>(htons(size_y));
    res.write_primitive<uint16_t>(htons(game_length));
    res.write_primitive<uint16_t>(htons(explosion_radius));
    res.write_primitive<uint16_t>(htons(bomb_timer));

    return res;
}

Buffer GameInfo::produce_accepted_player_message() const {
    Buffer res(128);
    res.write_primitive<uint8_t>(1);
    res.write_primitive<uint8_t>(added_player.player_id);
    res.serialize_string(added_player.name);
    res.serialize_string(added_player.address);

    return res;
}

Buffer GameInfo::produce_game_started_message() {
    Buffer res(256);
    res.write_primitive<uint8_t>(2);
    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(players.size())));
    for (auto &player : players) {
        res.write_primitive<uint8_t>(player.second.player_id);
        res.serialize_string(player.second.name);
        res.serialize_string(player.second.address);
    }

    return res;
}

Buffer GameInfo::produce_turn_message() {
    Buffer res(512);

    res.write_primitive<uint8_t>(3);
    res.write_primitive<uint16_t>(htons(current_turn));
    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(events.size())));
    for (auto &event_wrapper : events) {
        std::visit([&res](auto &wrapper) { wrapper.serialize(res); }, event_wrapper);
    }

    return res;
}

Buffer GameInfo::produce_game_ended_message() {
    Buffer res(128);

    res.write_primitive<uint8_t>(4);
    res.write_primitive<uint32_t>(htonl(static_cast<uint32_t>(scores.size())));
    for (auto &score : scores) {
        res.write_primitive<uint8_t>(score.first);
        res.write_primitive<uint32_t>(score.second);
    }

    return res;
}

void GameInfo::bomb_row_explosion(Position starting_position, std::set<Position> &blocks_exploded, BombExplodedEvent &event, uint8_t direction) {
    bool out_of_board = false;
    bool leave_after_next = false;

    for (uint16_t i = 0; i <= explosion_radius; i++) {
        auto players_set_it = players_position_reversed.find(starting_position);
        if (players_set_it != players_position_reversed.end()) {
            for (auto &player_id : players_set_it->second) {
                event.robot_destroyed(player_id);
                died_this_turn[player_id] = true;
            }
        }

        if (blocks.count(starting_position) > 0) {
            blocks_exploded.insert(starting_position);
            event.block_destroyed(starting_position);
            break;
        }

        if (leave_after_next) break;

        switch(direction) {
            case 0:
                starting_position.go_up();
                out_of_board = starting_position.y == size_y;
                break;
            case 1:
                starting_position.go_down();
                leave_after_next = starting_position.y == 0;
                break;
            case 2:
                starting_position.go_right();
                out_of_board = starting_position.x == size_x;
                break;
            case 3:
                starting_position.go_left();
                leave_after_next = starting_position.x == 0;
                break;
            default:
                break;
        }

        if (out_of_board)
            break;
    }
}

void GameInfo::compute_explosions() {
    std::set<Position> blocks_exploded;

    for (auto it = bombs.begin(); it != bombs.end();) {
        it->second.bomb_timer--;
        if (it->second.bomb_timer == 0) {
            BombExplodedEvent bomb_event(it->first);

            bomb_row_explosion(it->second.bomb_position, blocks_exploded, bomb_event, 0);
            bomb_row_explosion(it->second.bomb_position, blocks_exploded, bomb_event, 1);
            bomb_row_explosion(it->second.bomb_position, blocks_exploded, bomb_event, 2);
            bomb_row_explosion(it->second.bomb_position, blocks_exploded, bomb_event, 3);

            it = bombs.erase(it);
            events.emplace_back(bomb_event);
        }
        else {
            it++;
        }
    }

    for (auto &pos : blocks_exploded) {
        blocks.erase(pos);
    }

}
