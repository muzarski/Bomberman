#include "../../include/connection_handlers/udp_connection.h"
#include "../../include/connection_bridge.h"

#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

UdpConnection::UdpConnection(boost::asio::io_context &io_context_, const std::string &remote_ip,
                             const std::string &remote_port, uint16_t local_port) :
        socket(io_context_, udp::endpoint(udp::v6(), local_port)),
        receive_buffer(DATAGRAM_CAPACITY),
        io_context(io_context_) {

    udp::resolver resolver(io_context_);
    remote_endpoint = *resolver.resolve(remote_ip, remote_port).begin();
}

void UdpConnection::receive() {
    socket.async_receive(boost::asio::buffer(receive_buffer.get_data()),
                         boost::bind(&UdpConnection::handle_receive, this,
                                     boost::asio::placeholders::error,
                                     boost::asio::placeholders::bytes_transferred));
}

void UdpConnection::handle_receive(const boost::system::error_code &error, std::size_t read_len) {
    if (!error) {
        receive_buffer.reset_read_it();
        receive_buffer.set_length(read_len);
        bridge->from_gui(receive_buffer);
        receive();
    }
    else {
        bridge->notify_close_from_gui();
        close_connection();
    }
}

void UdpConnection::start_connection(ConnectionBridge *bridge_ptr) {
    bridge = bridge_ptr;
    receive();
}

void UdpConnection::write(Buffer &buffer) {
    boost::asio::post(io_context,
                      [this, buffer](){
        bool in_progress = !to_send_q.empty();
        to_send_q.push_back(buffer);
        if (!in_progress) {
            do_write();
        }
    });
}

void UdpConnection::do_write() {
    socket.async_send_to(boost::asio::buffer(to_send_q.front().get_data(), to_send_q.front().get_length()),
                         remote_endpoint,
                         [this](boost::system::error_code err, [[maybe_unused]] std::size_t write_len) {
        if (!err) {
            to_send_q.pop_front();
            if (!to_send_q.empty()) {
                do_write();
            }
        } else {
            bridge->notify_close_from_gui();
            close_connection();
        }
    });
}

void UdpConnection::close_connection() {
    boost::asio::post(io_context, [this]() { socket.close(); });
}
