#ifndef PARSER_H
#define PARSER_H

#include <optional>
#include "udp_structs.h"
#include "tcp_structs.h"

namespace Parser {
    using GuiRcvMessage = std::variant<PlaceBomb, PlaceBlock, Move>;
    using GuiRcvMessageOptional = std::optional<GuiRcvMessage>;
    using ServerRcvMessage = std::variant<Hello, AcceptedPlayer, GameStarted, Turn, GameEnded>;

     inline GuiRcvMessageOptional parse_input_message(Buffer &buffer) {
        static std::map<uint8_t, std::function<GuiRcvMessage(Buffer&)>> m = {
                {0, [](Buffer &buffer) { return PlaceBomb(buffer); }},
                {1, [](Buffer &buffer) { return PlaceBlock(buffer); }},
                {2, [](Buffer &buffer){ return Move(buffer); }}
        };

        try {
            auto message_id = buffer.read_primitive<uint8_t>();
            if(message_id <= 2) {
                return m[message_id](buffer);
            }
            return {};
        }
        catch (...) {
            return {};
        }
    }

    inline ServerRcvMessage get_server_message(Buffer &buffer) {
        static std::map<uint8_t, std::function<ServerRcvMessage(void)>> m = {
                {0, []() { return Hello(); }},
                {1, []() { return AcceptedPlayer(); }},
                {2, []() { return GameStarted(); }},
                {3, []() { return Turn(); }},
                {4, []() { return GameEnded(); }}
        };

        try {
            auto message_id = buffer.read_primitive<uint8_t>();
            if (message_id <= 4) {
                return m[message_id]();
            }
            throw ParsingException();
        }
        catch (...) {
            throw ParsingException();
        }
    }
};

#endif // PARSER_H
