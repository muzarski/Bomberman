#include "../../include/connection_handlers/tcp_server.h"

TcpServer::TcpServer(boost::asio::io_context &io_context_, uint16_t port, Args args)
    : io_context(io_context_),
      connection_acceptor(io_context_, tcp::endpoint(tcp::v6(), port)),
      robots_game(io_context_, std::move(args)) {
    do_accept();
}

void TcpServer::do_accept() {
    connection_acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::make_shared<TcpConnection>(std::move(socket), robots_game)->start();
        }

        do_accept();
    });
}