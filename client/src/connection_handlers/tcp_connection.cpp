#include "../../include/connection_handlers/tcp_connection.h"
#include "../../include/connection_bridge.h"

#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

TcpConnection::TcpConnection(boost::asio::io_context &io_context_, const std::string &remote_ip,
          const std::string &remote_port)
        :
        io_context(io_context_),
        socket(io_context),
        receive_buffer(1024),
        current_message() {

    tcp::resolver resolver(io_context);
    tcp::endpoint remote_endpoint = *resolver.resolve(remote_ip, remote_port).begin();
    socket.connect(remote_endpoint);
    socket.set_option(tcp::no_delay(true));
}

void TcpConnection::start_connection(ConnectionBridge *bridge_ptr) {
    bridge = bridge_ptr;
    receive_message();
}

void TcpConnection::close_connection() {
    boost::asio::post(io_context, [this]() { socket.close(); });
}

void TcpConnection::receive_message() {
    boost::asio::async_read(socket, boost::asio::buffer(receive_buffer.get_data(), 1),
                            [this](boost::system::error_code err, std::size_t read_len) {

        if (!err) {
            receive_buffer.reset_read_it();
            receive_buffer.set_length(read_len);

            fill();
        }
        else {
            std::cerr << "ERROR WHEN READING FROM THE SERVER! Closing the connection...\n";
            bridge->notify_close_from_server();
            close_connection();
        }
    });
}

void TcpConnection::fill() {
    try {
        current_message = Parser::get_server_message(receive_buffer);
    }
    catch (...) {
        std::cerr << "Received wrong message_id from server. Disconnecting...\n";
        bridge->notify_close_from_server();
        close_connection();
    }

    size_t next_bytes = std::visit([](auto &wrapper) { return wrapper.how_many_bytes(); }, current_message);
    receive_buffer.resize_if_needed(next_bytes);

    boost::asio::async_read(socket, boost::asio::buffer(receive_buffer.get_data(), next_bytes),
                            boost::bind(&TcpConnection::do_fill, this, boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));
}

void TcpConnection::do_fill(const boost::system::error_code &error, std::size_t read_len) {
    if (!error) {
        receive_buffer.set_length(read_len);
        receive_buffer.reset_read_it();

        size_t next_bytes = std::visit([this](auto &wrapper) { return wrapper.apply(receive_buffer); }, current_message);

        if (next_bytes == 0) {
            bridge->from_server(current_message);
            receive_message();
        }
        else {
            receive_buffer.resize_if_needed(next_bytes);
            boost::asio::async_read(socket, boost::asio::buffer(receive_buffer.get_data(), next_bytes),
                                    boost::bind(&TcpConnection::do_fill, this, boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
    }
    else {
        std::cerr << "ERROR WHEN READING FROM THE SERVER! Closing the connection...\n";
        bridge->notify_close_from_server();
        close_connection();
    }
}

void TcpConnection::write(Buffer &buffer) {
    boost::asio::post(io_context,
                      [this, buffer](){
        bool in_progress = !to_send_q.empty();
        to_send_q.push_back(buffer);
        if (!in_progress) {
            do_write();
        }
    });
}

void TcpConnection::do_write() {
    boost::asio::async_write(socket,
                             boost::asio::buffer(to_send_q.front().get_data(), to_send_q.front().get_length()),
                             [this](boost::system::error_code err, [[maybe_unused]] std::size_t write_len) {
        if (!err) {
            to_send_q.pop_front();
            if (!to_send_q.empty()) {
                do_write();
            }
        }
        else {
            std::cerr << "ERROR WHEN WRITING TO SERVER! Closing the connection...\n";
            bridge->notify_close_from_server();
            close_connection();
        }
    });
}
