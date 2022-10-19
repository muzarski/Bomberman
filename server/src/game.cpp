#include "../include/game.h"
#include "../include/connection_handlers/tcp_connection.h"
#include <iostream>

Game::Game(boost::asio::io_context &io_context_, Args &&args) :
    io_context(io_context_),
    turn_timer(io_context_),
    info(std::move(args)) {};

void Game::init_match() {
    info.current_turn = 0;
    info.events.clear();
    info.blocks.clear();
    info.bombs.clear();
    info.previous_turn_messages.clear();
    info.players_position_reversed.clear();
    info.players_position.clear();
    info.current_bomb_id = 0;
    info.died_this_turn.resize(info.players_count);

    for (uint8_t id = 0; id < info.players_count; ++id) {
        auto x = static_cast<uint16_t>(info.random() % info.size_x);
        auto y = static_cast<uint16_t>(info.random() % info.size_y);
        Position position(x, y);

        info.players_position.push_back(position);
        info.players_position_reversed[position].insert(id);
        info.events.emplace_back(PlayerMovedEvent(id, position));
    }

    for (uint16_t block = 0; block < info.initial_blocks; ++block) {
        auto x = static_cast<uint16_t>(info.random() % info.size_x);
        auto y = static_cast<uint16_t>(info.random() % info.size_y);
        Position position(x, y);

        if (!info.blocks.contains(position)) {
            info.blocks.insert(position);
            info.events.emplace_back(BlockPlacedEvent(position));
        }
    }
}

void Game::add_participant(TcpConnectionPtr participant) {
    std::string client_address = participant->get_client_address();
    participants[client_address] = participant;

    welcome(participant);
}

void Game::disconnect_participant(std::string &client_address) {
    participants.erase(client_address);
}

void Game::welcome(TcpConnectionPtr &participant) {
    Buffer buffer(0);
    buffer = info.produce_hello_message();
    participant->write_message(buffer);

    if (info.state == LOBBY) {
        for (auto &message : info.previous_accepted_player_messages) {
            participant->write_message(message);
        }
    }
    else {
        buffer = info.produce_game_started_message();
        participant->write_message(buffer);
        for (auto &message : info.previous_turn_messages) {
            participant->write_message(message);
        }
    }

    participant->receive_message();
}

void Game::handle_message(ClientRcvMessage &message) {
    if (info.state == LOBBY) {
        std::visit([this](auto &message) { message.update(info); }, message);
    }
    else {
        std::visit([this](auto &message) { message.add_move(players_move);}, message);
        return;
    }

    Buffer buffer(0);
    switch(info.what_message) {
        case ACCEPTED_PLAYER:
            buffer = info.produce_accepted_player_message();
            info.previous_accepted_player_messages.push_back(buffer);
            send_message_to_participants(buffer);
            break;

        case GAME_STARTED:
            init_match();
            buffer = info.produce_accepted_player_message();
            send_message_to_participants(buffer);

            buffer = info.produce_game_started_message();
            send_message_to_participants(buffer);

            buffer = info.produce_turn_message();
            info.previous_turn_messages.push_back(buffer);
            send_message_to_participants(buffer);

            prepare_turn();
            break;

        default:
            break;
    }
}

void Game::prepare_turn() {
    players_move.clear();
    info.events.clear();
    for (auto &&flag : info.died_this_turn) {
        flag = false;
    }

    turn_timer.expires_after(std::chrono::milliseconds(info.turn_duration));
    turn_timer.async_wait([this](const boost::system::error_code &ec) {
        if (!ec) {
            handle_turn();
        }
        else {
            std::cerr << "Timer error: " << ec.message() << std::endl;
            exit(1);
        }
    });
}

void Game::match_finished() {
    info.state = LOBBY;
    info.players.clear();
    info.players_joined = 0;
    info.previous_accepted_player_messages.clear();
    info.previous_turn_messages.clear();

    Buffer buffer = info.produce_game_ended_message();
    send_message_to_participants(buffer);
}

void Game::handle_turn() {
    info.compute_explosions();
    for (uint8_t id = 0; id < info.players_count; id++) {
        Player player = info.player_id_to_player_info[id];
        if (!info.died_this_turn[id] && players_move.contains(player.address)) {
            std::visit([this](auto &message) { message.update(info); }, players_move[player.address]);
        }
        else if (info.died_this_turn[id]) {
            Position death_position = info.players_position[id];
            info.players_position_reversed.erase(death_position);
            auto x = static_cast<uint16_t>(random() % info.size_x);
            auto y = static_cast<uint16_t>(random() % info.size_y);
            Position new_position(x, y);

            info.players_position[id] = new_position;
            info.players_position_reversed[new_position].insert(id);
            info.events.emplace_back(PlayerMovedEvent(id, new_position));
        }
    }
    info.current_turn++;

    Buffer buffer = info.produce_turn_message();
    info.previous_turn_messages.push_back(buffer);
    send_message_to_participants(buffer);

    if (info.current_turn == info.game_length) {
        match_finished();
    }
    else {
        prepare_turn();
    }
}

void Game::send_message_to_participants(Buffer &buffer) {
    for (auto &participant : participants) {
        participant.second->write_message(buffer);
    }
}