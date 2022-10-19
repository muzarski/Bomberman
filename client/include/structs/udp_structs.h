#ifndef UDP_STRUCTS_H
#define UDP_STRUCTS_H

#include "../../../common/include/buffer.h"

class PlaceBomb {
private:
    uint8_t message_id;

public:
    PlaceBomb(Buffer &buffer);
    Buffer server_message();
};

class PlaceBlock {
private:
    uint8_t message_id;

public:
    PlaceBlock(Buffer &buffer);
    Buffer server_message();
};

class Move {
private:
    uint8_t message_id;
    uint8_t direction;

public:
    Move(Buffer &buffer);
    Buffer server_message();
};

class Join {
private:
    uint8_t message_id;
    std::string name;

public:
    Join(std::string value);
    Buffer server_message() const;
};

#endif // UDP_STRUCTS_H
