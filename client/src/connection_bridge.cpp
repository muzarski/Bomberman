#include "../include/connection_bridge.h"

ConnectionBridge::ConnectionBridge(std::string &nickname, UdpConnection &gui_connection, TcpConnection &server_connection) :
    player_nickname(nickname),
    info(),
    had_to_disconnect(false),
    gui(gui_connection),
    server(server_connection) {

    gui.start_connection(this);
    server.start_connection(this);
};

void ConnectionBridge::from_gui(Buffer &buffer) {
    Parser::GuiRcvMessageOptional parsed = Parser::parse_input_message(buffer);
    if (parsed) {
        if (info.received_hello) {
            if(!info.sent_join) {
                info.sent_join = true;
                Buffer buf = Join(player_nickname).server_message();
                server.write(buf);
            }
            else {
                Buffer buf = std::visit([](auto &obj) { return obj.server_message(); }, parsed.value());
                server.write(buf);
            }
        }
    }
}

void ConnectionBridge::from_server(Parser::ServerRcvMessage &message) {
    std::visit([this](auto &wrapper) { wrapper.update(info); }, message);
    switch(info.action) {
        case SEND_LOBBY: {
            Buffer lobby = info.produce_lobby_message();
            gui.write(lobby);
            break;
        }

        case SEND_GAME: {
            Buffer game = info.produce_game_message();
            gui.write(game);
            break;
        }

        default:
            break;
    }
}

void ConnectionBridge::notify_close_from_server() {
    had_to_disconnect = true;
    gui.close_connection();
}

void ConnectionBridge::notify_close_from_gui() {
    had_to_disconnect = true;
    server.close_connection();
}

bool ConnectionBridge::disconnected() const {
    return had_to_disconnect;
}
