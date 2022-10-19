#ifndef TCP_STRUCTS_H
#define TCP_STRUCTS_H

#include <functional>
#include <variant>

#include "../../../common/include/common_structs.h"
#include "client_structs.h"

template<typename ...Types>
class TcpMessageWrapper {
private:
    size_t index;

public:
    TcpMessageWrapper() :
            index(0),
            my_fields({Types{}...}) {};

    explicit TcpMessageWrapper(std::vector<std::variant<Types...>> &&args) :
            index(0),
            my_fields(std::move(args)) {};

    std::vector<std::variant<Types...>> my_fields;

    size_t apply(Buffer &buffer) {
        size_t next_bytes = std::visit([&buffer](auto &wrapper) { return wrapper.apply(buffer); }, my_fields[index]);
        if (next_bytes == 0) {
            if (++index == my_fields.size()) {
                return 0;
            }
            next_bytes = std::visit([](auto &wrapper) { return wrapper.how_many_bytes(); }, my_fields[index]);
        }

        return next_bytes;
    }

    [[nodiscard]] size_t how_many_bytes() const {
        return std::visit([](auto &wrapper) { return wrapper.how_many_bytes(); }, my_fields[0]);
    }
};

template<typename T, typename U>
class PairWrapper {
private:
    TcpMessageWrapper<T, U> pair_wrapper;

public:
    PairWrapper() :
            pair_wrapper({
                T(), // key
                U()  // value
            }) {}


    size_t apply(Buffer &buffer) {
        return pair_wrapper.apply(buffer);
    }

    [[nodiscard]] size_t how_many_bytes() const {
        return pair_wrapper.how_many_bytes();
    }

    [[nodiscard]] const T& key() const {
        return std::get<T>(pair_wrapper.my_fields[0]);
    }

    [[nodiscard]] const U& value() const {
        return std::get<U>(pair_wrapper.my_fields[1]);
    }
};

template<typename T, typename U>
class MapWrapper {
private:
    U32Wrapper map_size;

    bool parsed_map_size;
    uint32_t currently_in_map;
    PairWrapper<T, U> currently_parsed;

public:
    MapWrapper() : map_size(), parsed_map_size(false), currently_in_map(0), currently_parsed(), value() {};

    std::map<T, U> value;

    size_t apply(Buffer &buffer) {
        if (!parsed_map_size) {
            map_size.apply(buffer);
            parsed_map_size = true;
            if (map_size.value != 0) {
                return currently_parsed.how_many_bytes();
            } else {
                return 0;
            }
        }

        size_t next_bytes = currently_parsed.apply(buffer);
        if (next_bytes == 0) {
            value[currently_parsed.key()] = currently_parsed.value();
            currently_in_map++;
            if (currently_in_map == map_size.value) {
                return 0;
            }

            currently_parsed = {};
            next_bytes = currently_parsed.how_many_bytes();
        }

        return next_bytes;
    }

    [[nodiscard]] size_t how_many_bytes() const {
        return map_size.how_many_bytes();
    }
};

class PlayerWrapper {
private:
    TcpMessageWrapper<StringWrapper> player_wrapper;

public:
    PlayerWrapper();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    [[nodiscard]] std::string name() const;
    [[nodiscard]] std::string address() const;

};

class PositionWrapper {
private:
    TcpMessageWrapper<U16Wrapper> position_wrapper;

public:
    PositionWrapper();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    [[nodiscard]] uint16_t x() const;
    [[nodiscard]] uint16_t y() const;

};

using HelloWrapper = TcpMessageWrapper<StringWrapper, U8Wrapper, U16Wrapper>;

class Hello {
private:
    HelloWrapper message_wrapper;

public:
    Hello();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

using AcceptedPlayerWrapper = TcpMessageWrapper<U8Wrapper, PlayerWrapper>;

class AcceptedPlayer {
private:
    AcceptedPlayerWrapper message_wrapper;

public:
    AcceptedPlayer();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

class GameStarted {
private:
    MapWrapper<U8Wrapper, PlayerWrapper> players;

public:
    GameStarted();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

using BombPlacedWrapper = TcpMessageWrapper<U32Wrapper, PositionWrapper>;

class BombPlaced {
private:
    BombPlacedWrapper message_wrapper;

public:
    BombPlaced();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

using BombExplodedWrapper = TcpMessageWrapper<U32Wrapper, ListWrapper<U8Wrapper>, ListWrapper<PositionWrapper>>;

class BombExploded {
private:
    BombExplodedWrapper message_wrapper;

    void compute_explosions(const Position &position, ClientInfo &info) const;

public:
    BombExploded();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

using PlayerMovedWrapper = TcpMessageWrapper<U8Wrapper, PositionWrapper>;

class PlayerMoved {
    PlayerMovedWrapper message_wrapper;

public:
    PlayerMoved();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

class BlockPlaced {
private:
    PositionWrapper position_wrapper;

public:
    BlockPlaced();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

using EventVariant = std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced>;

class EventWrapper {
private:
    U8Wrapper event_id;
    bool parsed_event_id;
    EventVariant event;

public:
    EventWrapper();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

using TurnWrapper = TcpMessageWrapper<U16Wrapper, ListWrapper<EventWrapper>>;

class Turn {
private:
    TurnWrapper message_wrapper;

public:
    Turn();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

class GameEnded {
private:
    MapWrapper<U8Wrapper, U32Wrapper> scores;

public:
    GameEnded();
    size_t apply(Buffer &buffer);
    [[nodiscard]] size_t how_many_bytes() const;
    void update(ClientInfo &info) const;

};

#endif // TCP_STRUCTS_H
