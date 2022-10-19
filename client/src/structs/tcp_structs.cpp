#include <iostream>

#include "../../include/structs/tcp_structs.h"

PlayerWrapper::PlayerWrapper() :
    player_wrapper({
        StringWrapper(),
        StringWrapper()
    }) {}

size_t PlayerWrapper::apply(Buffer &buffer) {
    return player_wrapper.apply(buffer);
}

size_t PlayerWrapper::how_many_bytes() const {
    return player_wrapper.how_many_bytes();
}

std::string PlayerWrapper::name() const {
    return std::visit([](auto &wrapper) { return wrapper.value; }, player_wrapper.my_fields[0]);
}

std::string PlayerWrapper::address() const {
    return std::visit([](auto &wrapper) { return wrapper.value; }, player_wrapper.my_fields[1]);
}

PositionWrapper::PositionWrapper() :
    position_wrapper({
        U16Wrapper(),
        U16Wrapper()
    }) {}

size_t PositionWrapper::apply(Buffer &buffer) {
    return position_wrapper.apply(buffer);
}

size_t PositionWrapper::how_many_bytes() const {
    return position_wrapper.how_many_bytes();
}

uint16_t PositionWrapper::x() const {
    return std::visit([](auto &wrapper) { return wrapper.value; }, position_wrapper.my_fields[0]);
}

uint16_t PositionWrapper::y() const {
    return std::visit([](auto &wrapper) { return wrapper.value; }, position_wrapper.my_fields[1]);
}

Hello::Hello() :
    message_wrapper({
        StringWrapper(), // server_name
        U8Wrapper(),     // player_count
        U16Wrapper(),    // size_x
        U16Wrapper(),    // size_y
        U16Wrapper(),    // game_length
        U16Wrapper(),    // explosion_radius
        U16Wrapper()     // bomb_timer
    }) {}

size_t Hello::apply(Buffer &buffer) {
    return message_wrapper.apply(buffer);
}

size_t Hello::how_many_bytes() const {
    return message_wrapper.how_many_bytes();
}

void Hello::update(ClientInfo &info) const {
    info.received_hello = true;
    info.action = SEND_LOBBY;
    info.server_name = std::get<StringWrapper>(message_wrapper.my_fields[0]).value;
    info.players_count = std::get<U8Wrapper>(message_wrapper.my_fields[1]).value;
    info.size_x = std::get<U16Wrapper>(message_wrapper.my_fields[2]).value;
    info.size_y = std::get<U16Wrapper>(message_wrapper.my_fields[3]).value;
    info.game_length = std::get<U16Wrapper>(message_wrapper.my_fields[4]).value;
    info.explosion_radius = std::get<U16Wrapper>(message_wrapper.my_fields[5]).value;
    info.bomb_timer = std::get<U16Wrapper>(message_wrapper.my_fields[6]).value;
}


AcceptedPlayer::AcceptedPlayer() :
    message_wrapper({
        U8Wrapper(),    // player_id
        PlayerWrapper() // player
    }) {};

size_t AcceptedPlayer::apply(Buffer &buffer) {
    return message_wrapper.apply(buffer);
}

size_t AcceptedPlayer::how_many_bytes() const {
    return message_wrapper.how_many_bytes();
}

void AcceptedPlayer::update(ClientInfo &info) const {
    info.action = SEND_LOBBY;
    uint8_t player_id = std::get<U8Wrapper>(message_wrapper.my_fields[0]).value;
    std::string player_name = std::get<PlayerWrapper>(message_wrapper.my_fields[1]).name();
    std::string player_address = std::get<PlayerWrapper>(message_wrapper.my_fields[1]).address();

    info.players[player_id] = Player{player_name, player_address};
}

GameStarted::GameStarted() :
    players() {};

size_t GameStarted::apply(Buffer &buffer) {
    return players.apply(buffer);
}

size_t GameStarted::how_many_bytes() const {
    return players.how_many_bytes();
}

void GameStarted::update(ClientInfo &info) const {
    info.scores.clear();
    info.action = DO_NOT_SEND;
    info.players.clear();
    auto &map_wrapper = players.value;
    for (auto &key_value : map_wrapper) {
        uint8_t player_id = key_value.first.value;
        std::string player_name = key_value.second.name();
        std::string player_address = key_value.second.address();

        info.players[player_id] = Player{player_name, player_address};
        info.scores[player_id] = 0;
    }
}

BombPlaced::BombPlaced() :
    message_wrapper({
        U32Wrapper(),     // bomb_id
        PositionWrapper() // bomb_position
    }) {}


size_t BombPlaced::apply(Buffer &buffer) {
    return message_wrapper.apply(buffer);
}

size_t BombPlaced::how_many_bytes() const {
    return message_wrapper.how_many_bytes();
}

void BombPlaced::update(ClientInfo &info) const {
    uint32_t bomb = std::get<U32Wrapper>(message_wrapper.my_fields[0]).value;
    uint16_t x = std::get<PositionWrapper>(message_wrapper.my_fields[1]).x();
    uint16_t y = std::get<PositionWrapper>(message_wrapper.my_fields[1]).y();

    info.bombs[bomb] = Bomb{{x, y}, info.bomb_timer};
}

BombExploded::BombExploded() :
    message_wrapper() {}

size_t BombExploded::apply(Buffer &buffer) {
    return message_wrapper.apply(buffer);
}

size_t BombExploded::how_many_bytes() const {
    return message_wrapper.how_many_bytes();
}

void BombExploded::compute_explosions(const Position &bomb_position, ClientInfo &info) const {
    for (uint16_t i = 0; i <= info.explosion_radius; i++) {
        uint16_t current_x = static_cast<uint16_t>(bomb_position.x + i);
        Position current_position{current_x, bomb_position.y};
        info.explosions.insert(current_position);
        if (current_x == info.size_x - 1 || info.blocks_copy.count(current_position) > 0) break;
    }

    for (uint16_t i = 0; i <= info.explosion_radius; i++) {
        uint16_t current_x = static_cast<uint16_t>(bomb_position.x - i);
        Position current_position{current_x, bomb_position.y};
        info.explosions.insert(current_position);
        if (current_x == 0 || info.blocks_copy.count(current_position) > 0) break;
    }

    for (uint16_t i = 0; i <= info.explosion_radius; i++) {
        uint16_t current_y = static_cast<uint16_t>(bomb_position.y + i);
        Position current_position{bomb_position.x, current_y};
        info.explosions.insert(current_position);
        if (current_y == info.size_y - 1 || info.blocks_copy.count(current_position) > 0) break;
    }

    for (uint16_t i = 0; i <= info.explosion_radius; i++) {
        uint16_t current_y = static_cast<uint16_t>(bomb_position.y - i);
        Position current_position{bomb_position.x, current_y};
        info.explosions.insert(current_position);
        if (current_y == 0 || info.blocks_copy.count(current_position) > 0) break;
    }
}

void BombExploded::update(ClientInfo &info) const {
    uint32_t bomb = std::get<U32Wrapper>(message_wrapper.my_fields[0]).value;
    const std::vector<U8Wrapper> &robots = std::get<ListWrapper<U8Wrapper>>(message_wrapper.my_fields[1]).value;
    const std::vector<PositionWrapper> &blocks = std::get<ListWrapper<PositionWrapper>>(message_wrapper.my_fields[2]).value;

    compute_explosions(info.bombs[bomb].bomb_position, info);
    info.bombs.erase(bomb);

    for (auto &player_id : robots) {
        if (!info.already_dead[player_id.value]) {
            info.already_dead[player_id.value] = true;
            info.scores[player_id.value]++;
        }
    }

    for (auto &pos: blocks) {
        info.blocks.erase({pos.x(), pos.y()});
    }
}

PlayerMoved::PlayerMoved() :
    message_wrapper() {}

size_t PlayerMoved::apply(Buffer &buffer) {
    return message_wrapper.apply(buffer);
}

size_t PlayerMoved::how_many_bytes() const {
    return message_wrapper.how_many_bytes();
}

void PlayerMoved::update(ClientInfo &info) const {
    uint8_t player = std::get<U8Wrapper>(message_wrapper.my_fields[0]).value;
    uint16_t x = std::get<PositionWrapper>(message_wrapper.my_fields[1]).x();
    uint16_t y = std::get<PositionWrapper>(message_wrapper.my_fields[1]).y();

    info.player_positions[player] = {x, y};
}


BlockPlaced::BlockPlaced() :
    position_wrapper() {}

size_t BlockPlaced::apply(Buffer &buffer) {
    return position_wrapper.apply(buffer);
}

size_t BlockPlaced::how_many_bytes() const {
    return position_wrapper.how_many_bytes();
}

void BlockPlaced::update(ClientInfo &info) const {
    uint16_t x = position_wrapper.x();
    uint16_t y = position_wrapper.y();

    info.blocks.insert({x, y});
}

EventWrapper::EventWrapper() :
    event_id(),
    parsed_event_id(false),
    event() {};

size_t EventWrapper::apply(Buffer &buffer) {
    static std::map<uint8_t, std::function<EventVariant(void)>> possible_events = {
            {0, []() { return BombPlaced(); }},
            {1, []() { return BombExploded(); }},
            {2, [](){ return PlayerMoved(); }},
            {3, []() { return BlockPlaced(); }}
    };

    if (!parsed_event_id) {
        event_id.apply(buffer);
        parsed_event_id = true;
        if (event_id.value > 3) {
            throw ParsingException();
        }

        event = possible_events[event_id.value]();
        return std::visit([](auto &wrapper) { return wrapper.how_many_bytes(); }, event);
    }

    return std::visit([&buffer](auto &wrapper) { return wrapper.apply(buffer); }, event);
}

size_t EventWrapper::how_many_bytes() const {
    return event_id.how_many_bytes();
}

void EventWrapper::update(ClientInfo &info) const {
    std::visit([&info](auto &wrapper) { wrapper.update(info); }, event);
}


Turn::Turn() :
    message_wrapper({
        U16Wrapper(),               // turn
        ListWrapper<EventWrapper>() // events
    }) {}

size_t Turn::apply(Buffer &buffer) {
    return message_wrapper.apply(buffer);
}

size_t Turn::how_many_bytes() const {
    return message_wrapper.how_many_bytes();
}

void Turn::update(ClientInfo &info) const {
    info.action = SEND_GAME;
    info.blocks_copy = info.blocks;
    for (auto &player : info.players) {
        info.already_dead[player.first] = false;
    }
    info.explosions.clear();
    for (auto &bomb : info.bombs) {
        bomb.second.bomb_timer--;
    }
    info.turn = std::get<U16Wrapper>(message_wrapper.my_fields[0]).value;
    const std::vector<EventWrapper> &list = std::get<ListWrapper<EventWrapper>>(message_wrapper.my_fields[1]).value;
    for (auto &el : list) {
        el.update(info);
    }
}


GameEnded::GameEnded() :
    scores() {};

size_t GameEnded::apply(Buffer &buffer) {
    return scores.apply(buffer);
}

size_t GameEnded::how_many_bytes() const {
    return scores.how_many_bytes();
}

void GameEnded::update(ClientInfo &info) const {
    info.action = SEND_LOBBY;
    info.players.clear();
    info.scores.clear();
    info.blocks.clear();
    info.bombs.clear();
    info.sent_join = false;
}
