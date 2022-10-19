#include "../../include/structs/tcp_structs.h"

#include <utility>
#include <iostream>

Join::Join() = default;
Join::Join(std::string client_address_) :
    name(),
    client_address(std::move(client_address_)) {}

size_t Join::apply(Buffer &buffer) {
    return name.apply(buffer);
}

size_t Join::how_many_bytes() const {
    return name.how_many_bytes();
}

void Join::update(GameInfo &info) {
    if (info.state == LOBBY && !info.players.contains(client_address)) {
        Player new_player(info.players_joined, name.value, client_address);
        info.added_player = new_player;
        info.players[client_address] = info.added_player;
        info.player_id_to_player_info[info.players_joined++] = new_player;
        if (info.players_joined == info.players_count) {
            info.state = GAME;
            info.what_message = GAME_STARTED;
        }
        else {
            info.what_message = ACCEPTED_PLAYER;
        }

    }
    else {
        info.what_message = NOTHING;
    }
}

void Join::add_move(std::map<std::string, ClientRcvMessage> &players_move) {
    players_move[client_address] = *this;
}

PlaceBomb::PlaceBomb() = default;
PlaceBomb::PlaceBomb(std::string client_address_) :
    client_address(std::move(client_address_)) {};

size_t PlaceBomb::apply([[maybe_unused]] Buffer &buffer) {
    return 0;
}

size_t PlaceBomb::how_many_bytes() const {
    return 0;
}

void PlaceBomb::update(GameInfo &info) {
    if (info.state == GAME && info.players.count(client_address) > 0) {
        Position player_position = info.players_position[info.players[client_address].player_id];
        info.bombs[info.current_bomb_id] = {player_position, info.bomb_timer};
        info.events.emplace_back(BombPlacedEvent(info.current_bomb_id++, player_position));
    }
    else {
        info.what_message = NOTHING;
    }
}

void PlaceBomb::add_move(std::map<std::string, ClientRcvMessage> &players_move) {
    players_move[client_address] = *this;
}

PlaceBlock::PlaceBlock() = default;
PlaceBlock::PlaceBlock(std::string client_address_) :
    client_address(std::move(client_address_)) {};

size_t PlaceBlock::apply([[maybe_unused]] Buffer &buffer) {
    return 0;
}

size_t PlaceBlock::how_many_bytes() const {
    return 0;
}

void PlaceBlock::update(GameInfo &info) const {
    if (info.state == GAME && info.players.count(client_address) > 0) {
        Position player_position = info.players_position[info.players[client_address].player_id];
        if (info.blocks.count(player_position) == 0) {
            info.blocks.insert(player_position);
            info.events.emplace_back(BlockPlacedEvent(player_position));
        }
    }
    else {
        info.what_message = NOTHING;
    }
}

void PlaceBlock::add_move(std::map<std::string, ClientRcvMessage> &players_move) {
    players_move[client_address] = *this;
}

Move::Move() = default;
Move::Move(std::string client_address_) :
    direction(),
    client_address(std::move(client_address_)) {};

size_t Move::apply(Buffer &buffer) {
    return direction.apply(buffer);
}

size_t Move::how_many_bytes() const {
    return direction.how_many_bytes();
}

void Move::update(GameInfo &info) {
    if (info.state == GAME && info.players.count(client_address) > 0) {
        uint8_t player_id = info.players[client_address].player_id;
        Position player_position = info.players_position[player_id];
        std::optional<Position> new_position = info.move_if_valid(player_position, direction.value);
        if (new_position.has_value()) {
            info.players_position_reversed[player_position].erase(player_id);
            info.players_position_reversed[new_position.value()].insert(player_id);
            info.players_position[player_id] = new_position.value();
            info.events.emplace_back(PlayerMovedEvent(player_id, new_position.value()));
        }
    }
    else {
        info.what_message = NOTHING;
    }
}

void Move::add_move(std::map<std::string, ClientRcvMessage> &players_move) {
    players_move[client_address] = *this;
}

