#include <iostream>

#include "../../include/structs/udp_structs.h"

PlaceBomb::PlaceBomb(Buffer &buffer) :
        message_id(0) {
    if (buffer.has_trailing_data()) {
        throw ParsingException();
    }
}

Buffer PlaceBomb::server_message() {
    Buffer res(1);
    res.write_primitive<uint8_t>(++message_id);
    return res;
}

PlaceBlock::PlaceBlock(Buffer &buffer) :
        message_id(1) {
    if (buffer.has_trailing_data()) {
        throw ParsingException();
    }
};

Buffer PlaceBlock::server_message() {
    Buffer res(1);
    res.write_primitive<uint8_t>(++message_id);
    return res;
}

Move::Move(Buffer &buffer) :
        message_id(2) {
    try {
        direction = buffer.read_primitive<uint8_t>();
    }
    catch (...) {
        throw ParsingException();
    }

    if (buffer.has_trailing_data() || direction > 3) {
        throw ParsingException();
    }
}

Buffer Move::server_message() {
    Buffer res(2);
    res.write_primitive<uint8_t>(++message_id);
    res.write_primitive<uint8_t>(direction);
    return res;
}

Join::Join(std::string value) :
    message_id(0),
    name(std::move(value)) {}

Buffer Join::server_message() const {
    Buffer res(2 * sizeof(uint8_t) + name.length());
    res.write_primitive<uint8_t>(message_id);
    res.serialize_string(name);
    return res;
}