#ifndef CONNECTION_BRIDGE_H
#define CONNECTION_BRIDGE_H

#include <iostream>
#include <variant>

#include "structs/parser.h"
#include "connection_handlers/tcp_connection.h"
#include "connection_handlers/udp_connection.h"

class ConnectionBridge : public std::enable_shared_from_this<ConnectionBridge> {
private:
    std::string player_nickname;
    ClientInfo info;

    bool had_to_disconnect;
    UdpConnection &gui;
    TcpConnection &server;

public:
    ConnectionBridge(std::string &nickname, UdpConnection &gui_connection, TcpConnection &server_connection);
    void from_gui(Buffer &buffer);
    void from_server(Parser::ServerRcvMessage &message);
    void notify_close_from_server();
    void notify_close_from_gui();
    bool disconnected() const;
};

#endif // CONNECTION_BRIDGE_H
