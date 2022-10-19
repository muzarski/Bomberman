#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <cstring>
#include <map>

class BufferException : public std::exception {
    [[nodiscard]] const char* what() const noexcept override {
        return "BufferException";
    }
};

class ParsingException : public std::exception {
    [[nodiscard]] const char* what() const noexcept override {
        return "ParsingException";
    }
};

[[maybe_unused]] const size_t DATAGRAM_CAPACITY = 65507;

class Buffer {
private:
    std::vector<char> buffer_data;
    size_t length;
    size_t read_it;

    bool readable(size_t read_len);

public:
    explicit Buffer(size_t size);
    void resize_if_needed(size_t write_size);

    template<typename T>
    void write_primitive(const T value) {
        resize_if_needed(sizeof(T));
        memcpy(buffer_data.data() + length, &value, sizeof(T));
        length += sizeof(T);
    }

    template<typename T>
    T read_primitive() {
        if (!readable(sizeof(T))) {
            throw BufferException();
        }

        T value;
        memcpy(&value, buffer_data.data() + read_it, sizeof(T));
        read_it += sizeof(T);
        return value;
    }

    void write_string(const std::string &value);
    std::string read_string(size_t len);
    void serialize_string(const std::string &value);

    std::vector<char>& get_data();
    [[nodiscard]] bool has_trailing_data() const;
    void set_length(size_t len);
    void reset_read_it();
    [[nodiscard]] size_t get_length() const;
};

#endif // BUFFER_H
