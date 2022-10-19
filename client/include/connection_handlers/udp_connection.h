#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <iostream>
#include <deque>
#include <boost/asio.hpp>

#include "../../../common/include/buffer.h"

using boost::asio::ip::udp;

class ConnectionBridge;

class UdpConnection {
private:
    udp::socket socket;
    Buffer receive_buffer;
    ConnectionBridge *bridge;
    std::deque<Buffer> to_send_q;
    boost::asio::io_context &io_context;
    udp::endpoint remote_endpoint;

    void receive();
    void handle_receive(const boost::system::error_code &error, std::size_t read_len);
    void do_write();

public:
    UdpConnection(boost::asio::io_context &io_context, const std::string &remote_ip,
                  const std::string &remote_port, uint16_t local_port);

    void start_connection(ConnectionBridge *bridge_ptr);
    void write(Buffer &buffer);
    void close_connection();
};

#endif // UDP_SERVER_H
