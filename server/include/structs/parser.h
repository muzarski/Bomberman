#ifndef PARSER_H
#define PARSER_H

#include <variant>
#include <functional>
#include "tcp_structs.h"

namespace Parser {
    inline ClientRcvMessage get_client_message(Buffer &buffer, std::string &client_address) {
        std::vector<std::function<ClientRcvMessage(void)>> m = {
                {[&client_address]() { return Join(client_address); }},
                {[&client_address]() { return PlaceBomb(client_address); }},
                {[&client_address]() { return PlaceBlock(client_address); }},
                {[&client_address]() { return Move(client_address); }}
        };

        try {
            auto message_id = buffer.read_primitive<uint8_t>();
            if (message_id <= 3) {
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

