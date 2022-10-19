#ifndef TCP_STRUCTS_H
#define TCP_STRUCTS_H

#include "server_structs.h"

class Join;
class PlaceBomb;
class PlaceBlock;
class Move;

using ClientRcvMessage = std::variant<Join, PlaceBomb, PlaceBlock, Move>;

class Join {
private:
    StringWrapper name;

public:
    std::string client_address;

    Join();
    explicit Join(std::string client_address_);
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(GameInfo &info);
    void add_move(std::map<std::string, ClientRcvMessage> &players_move);
};

class PlaceBomb {
public:
    std::string client_address;

    PlaceBomb();
    explicit PlaceBomb(std::string client_address_);
    size_t apply([[maybe_unused]] Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(GameInfo &info);
    void add_move(std::map<std::string, ClientRcvMessage> &players_move);
};

class PlaceBlock {
public:
    std::string client_address;

    PlaceBlock();
    explicit PlaceBlock(std::string client_address_);
    size_t apply([[maybe_unused]] Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(GameInfo &info) const;
    void add_move(std::map<std::string, ClientRcvMessage> &players_move);
};

class Move {
private:
    U8Wrapper direction;

public:
    std::string client_address;

    Move();
    explicit Move(std::string client_address_);
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(GameInfo &info);
    void add_move(std::map<std::string, ClientRcvMessage> &players_move);
};

#endif // TCP_STRUCTS_H
