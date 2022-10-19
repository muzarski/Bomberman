#include "../../include/connection_handlers/tcp_connection.h"

#include <iostream>
#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

TcpConnection::TcpConnection(tcp::socket socket_, Game &game_) :
        socket(std::move(socket_)),
        game(game_),
        receive_buffer(128),
        current_message(Join("")),
        send_buffer(256) {

    socket.set_option(tcp::no_delay(true));
    std::stringstream str_stream;
    str_stream << socket.remote_endpoint();
    client_address = str_stream.str();
}

void TcpConnection::start() {
    game.add_participant(shared_from_this());
}

void TcpConnection::receive_message() {
    boost::asio::async_read(socket, boost::asio::buffer(receive_buffer.get_data(), 1),
        [this](boost::system::error_code err, std::size_t read_len) {

            if (!err) {
                receive_buffer.reset_read_it();
                receive_buffer.set_length(read_len);

                try {
                    current_message = Parser::get_client_message(receive_buffer, client_address);
                }
                catch (...) {
                    std::cerr << "Received wrong message_id from the client " << client_address << ". Closing the connection...\n";
                    close_connection();
                }

                size_t next_bytes = std::visit([](auto &wrapper) { return wrapper.how_many_bytes(); }, current_message);

                if (next_bytes == 0) {
                    game.handle_message(current_message);
                    receive_message();
                }
                else {
                    receive_buffer.resize_if_needed(next_bytes);
                    boost::asio::async_read(socket, boost::asio::buffer(receive_buffer.get_data(), next_bytes),
                                            boost::bind(&TcpConnection::fill_message, this, boost::asio::placeholders::error,
                                                        boost::asio::placeholders::bytes_transferred));
                }

            }
            else if (err == boost::asio::error::eof) {
                std::cerr << "Client " << client_address << " disconnected.\n";
                close_connection();
            }
            else {
                std::cerr << "ERROR WHEN READING FROM THE client " << client_address << "! Closing the connection...\n";
                close_connection();
            }
        });
}

void TcpConnection::fill_message(const boost::system::error_code &error, std::size_t read_len) {
    if (!error) {
        receive_buffer.set_length(read_len);
        receive_buffer.reset_read_it();

        size_t next_bytes = std::visit([this](auto &wrapper) { return wrapper.apply(receive_buffer); }, current_message);

        if (next_bytes == 0) {
            game.handle_message(current_message);
            receive_message();
        }
        else {
            receive_buffer.resize_if_needed(next_bytes);
            boost::asio::async_read(socket, boost::asio::buffer(receive_buffer.get_data(), next_bytes),
                                    boost::bind(&TcpConnection::fill_message, this, boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
    }
    else if (error == boost::asio::error::eof) {
        std::cerr << "Client " << socket.remote_endpoint() << " disconnected.\n";
        close_connection();
    }
    else {
        std::cerr << "ERROR WHEN READING FROM THE client " << client_address << "! Closing the connection...\n";
        close_connection();
    }
}

void TcpConnection::write_message(Buffer &buffer) {
      bool in_progress = !to_send_q.empty();
      to_send_q.push_back(buffer);
      if (!in_progress) {
          do_write_message();
      }
}

void TcpConnection::do_write_message() {
    boost::asio::async_write(socket,
                        boost::asio::buffer(to_send_q.front().get_data(), to_send_q.front().get_length()),
         [this](boost::system::error_code err, [[maybe_unused]] std::size_t write_len) {
             if (!err) {
                 to_send_q.pop_front();
                 if (!to_send_q.empty()) {
                     do_write_message();
                 }
             }
             else if (err == boost::asio::error::eof) {
                 std::cerr << "Client " << socket.remote_endpoint() << " disconnected.\n";
                 close_connection();
             }
             else {
                 std::cerr << "ERROR WHEN READING FROM THE client " << socket.remote_endpoint() << "! Closing the connection...\n";
                 close_connection();
             }
         });
}

void TcpConnection::close_connection() {
    game.disconnect_participant(client_address);
}

std::string TcpConnection::get_client_address() const {
    return client_address;
}