#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <deque>
#include <boost/asio.hpp>
#include "../../../common/include/buffer.h"
#include "../structs/server_structs.h"
#include "../structs/parser.h"
#include "../game.h"

using boost::asio::ip::tcp;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
private:
    tcp::socket socket;
    Game &game;
    Buffer receive_buffer;
    ClientRcvMessage current_message;
    Buffer send_buffer;
    std::string client_address;
    std::deque<Buffer> to_send_q;

    void fill_message(const boost::system::error_code &error, std::size_t read_len);
    void do_write_message();
    void close_connection();

public:
    TcpConnection(tcp::socket socket_, Game &game_);
    void start();
    [[nodiscard]] std::string get_client_address() const;
    void receive_message();
    void write_message(Buffer &buffer);
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

#endif // TCP_CONNECTION_H
