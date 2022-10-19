#include <iostream>

#include "../include/common_structs.h"

U8Wrapper::U8Wrapper() :
    value() {}

size_t U8Wrapper::apply(Buffer &buffer) {
    value = buffer.read_primitive<uint8_t>();
    return 0;
}

size_t U8Wrapper::how_many_bytes() const {
    return sizeof(uint8_t);
}

U16Wrapper::U16Wrapper() :
    value() {}

size_t U16Wrapper::apply(Buffer &buffer) {
    value = ntohs(buffer.read_primitive<uint16_t>());
    return 0;
}

size_t U16Wrapper::how_many_bytes() const {
    return sizeof(uint16_t);
}

U32Wrapper::U32Wrapper() :
    value() {}

size_t U32Wrapper::apply(Buffer &buffer) {
    value = ntohl(buffer.read_primitive<uint32_t>());
    return 0;
}

size_t U32Wrapper::how_many_bytes() const {
    return sizeof(uint32_t);
}


StringWrapper::StringWrapper() : string_len(), written_string_len(false), value() {}

size_t StringWrapper::apply(Buffer &buffer) {
    if (written_string_len) {
        value = buffer.read_string(string_len.value);
        return 0;
    }

    string_len.apply(buffer);
    written_string_len = true;

    return string_len.value;
}

size_t StringWrapper::how_many_bytes() const {
    return sizeof(uint8_t);
}
