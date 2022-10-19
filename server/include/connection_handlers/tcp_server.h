#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <boost/asio.hpp>
#include "../game.h"
#include "tcp_connection.h"

using boost::asio::ip::tcp;

class TcpServer {
private:
    boost::asio::io_context &io_context;
    tcp::acceptor connection_acceptor;
    Game robots_game;

    void do_accept();

public:
    TcpServer(boost::asio::io_context &io_context_, uint16_t port, Args args);

};

#endif // TCP_SERVER_H
