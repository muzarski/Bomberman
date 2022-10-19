#ifndef GAME_H
#define GAME_H

#include <set>
#include <map>
#include <random>
#include <boost/asio.hpp>
#include "structs/server_structs.h"
#include "structs/parser.h"

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using boost::asio::ip::tcp;

class Game {
private:
    boost::asio::io_context &io_context;
    boost::asio::steady_timer turn_timer;
    GameInfo info;
    std::map<std::string, ClientRcvMessage> players_move;
    std::map<std::string, TcpConnectionPtr> participants;

    void init_match();
    void send_message_to_participants(Buffer &buffer);
    void prepare_turn();
    void handle_turn();
    void match_finished();
    void welcome(TcpConnectionPtr &participant);

public:
    Game(boost::asio::io_context &io_context_, Args &&args);
    void add_participant(TcpConnectionPtr participant);
    void disconnect_participant(std::string &client_address);
    void handle_message(ClientRcvMessage &message);
};

#endif // GAME_H
