#ifndef COMMON_STRUCTS_H
#define COMMON_STRUCTS_H

#include <set>
#include <netinet/in.h>

#include "buffer.h"

//struct Player {
//    std::string name;
//    std::string address;
//};
//
//struct Position {
//    uint16_t x;
//    uint16_t y;
//};
//
//struct PositionComparator {
//    bool operator() (const Position &a, const Position &b) const {
//        if (a.x == b.x) {
//            return a.y < b.y;
//        }
//        return a.x < b.x;
//    }
//};

//struct Bomb {
//    Position position;
//    uint16_t timer;
//};

struct Player {
    uint8_t player_id{};
    std::string name;
    std::string address;

    Player() = default;
    Player(uint8_t player_id_, std::string name_, std::string address_) :
            player_id(player_id_),
            name(std::move(name_)),
            address(std::move(address_)) {};
    Player(std::string name_, std::string address_) :
            player_id(0),
            name(std::move(name_)),
            address(std::move(address_)) {};
};

struct Position {
    uint16_t x{};
    uint16_t y{};

    Position() = default;
    Position(uint16_t x_, uint16_t y_) :
            x(x_),
            y(y_) {};

    bool operator<(const Position &other) const {
        return std::tie(x, y) < std::tie(other.x, other.y);
    }

    void go_up() { y++; }
    void go_down() { y--; }
    void go_right() { x++; }
    void go_left() { x--; }
};

struct Bomb {
    Position bomb_position{};
    uint16_t bomb_timer{};

    Bomb() = default;
    Bomb(Position bomb_position_, uint16_t bomb_timer_) :
            bomb_position(bomb_position_),
            bomb_timer(bomb_timer_) {};
};

class U8Wrapper {
public:
    uint8_t value;

    U8Wrapper();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;

    bool operator<(const U8Wrapper &other) const {
        return value < other.value;
    }
};

class U16Wrapper {
public:
    uint16_t value;

    U16Wrapper();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
};

class U32Wrapper {
public:
    uint32_t value;

    U32Wrapper();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
};

class StringWrapper {
private:
    U8Wrapper string_len;
    bool written_string_len;

public:
    std::string value;

    StringWrapper();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
};

template<typename T>
class ListWrapper {
private:
    U32Wrapper list_len;
    bool parsed_list_len;
    uint32_t currently_in_list;
    T currently_parsed;

public:
    ListWrapper() : list_len(), parsed_list_len(false), currently_in_list(0), currently_parsed(), value() {};

    std::vector<T> value;

    size_t apply(Buffer &buffer) {
        if (!parsed_list_len) {
            list_len.apply(buffer);
            parsed_list_len = true;
            if (list_len.value != 0) {
                return currently_parsed.how_many_bytes();
            }
            else {
                return 0;
            }
        }

        size_t next_bytes = currently_parsed.apply(buffer);
        if (next_bytes == 0) {
            value.push_back(currently_parsed);
            currently_parsed = {};
            currently_in_list++;

            if (currently_in_list == list_len.value) {
                return 0;
            }

            next_bytes = currently_parsed.how_many_bytes();
        }

        return next_bytes;
    }

    [[nodiscard]] size_t how_many_bytes() const {
        return list_len.how_many_bytes();
    }
};

#endif // COMMON_STRUCTS_H
