#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <iostream>
#include <deque>
#include <boost/asio.hpp>

#include "../structs/parser.h"
#include "../../../common/include/buffer.h"
#include "../../../common/include/common_structs.h"

using boost::asio::ip::tcp;

class ConnectionBridge;

class TcpConnection {
private:
    boost::asio::io_context &io_context;
    tcp::socket socket;
    Buffer receive_buffer;
    ConnectionBridge *bridge;
    std::deque<Buffer> to_send_q;
    Parser::ServerRcvMessage current_message;

    void do_write();
    void fill();
    void do_fill(const boost::system::error_code &error, std::size_t read_len);

public:
    TcpConnection(boost::asio::io_context &io_context_, const std::string &remote_ip,
                  const std::string &remote_port);

    void start_connection(ConnectionBridge *bridge_ptr);
    void write(Buffer &buffer);
    void receive_message();
    void close_connection();
};


#endif // TCP_CLIENT_H
