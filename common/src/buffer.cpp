#include "../include/buffer.h"

Buffer::Buffer(size_t size) : length(0), read_it(0) {
    buffer_data.resize(size);
}

bool Buffer::readable(size_t read_len) {
    return read_it + read_len <= length;
}

void Buffer::resize_if_needed(size_t write_size) {
    if (length + write_size > buffer_data.size()) {
        buffer_data.resize(2 * buffer_data.size());
    }
}

void Buffer::write_string(const std::string &value) {
    resize_if_needed(value.length());
    for (char i : value) {
        write_primitive<char>(i);
    }
}

std::string Buffer::read_string(size_t len) {
    if (!readable(len))
        throw BufferException();

    std::string value(len, 0);
    for (size_t i = 0; i < len; i++) {
        value[i] = read_primitive<char>();
    }
    return value;
}

void Buffer::serialize_string(const std::string &value) {
    write_primitive<uint8_t>(static_cast<uint8_t>(value.length()));
    write_string(value);
}

std::vector<char>& Buffer::get_data() {
    return buffer_data;
}

bool Buffer::has_trailing_data() const {
    return read_it < length;
}

void Buffer::set_length(size_t len) {
    length = len;
}

void Buffer::reset_read_it() {
    read_it = 0;
}

size_t Buffer::get_length() const {
    return length;
}