#pragma once
#include <cstdint>
#include <string>

class Base64 {
public:
    static std::string encode(const uint8_t* data, size_t len) {
        static const char table[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string result;
        result.reserve(4 * ((len + 2) / 3));

        for (size_t i = 0; i < len; i += 3) {
            uint32_t n = uint32_t(data[i]) << 16;
            if (i + 1 < len) n |= uint32_t(data[i + 1]) << 8;
            if (i + 2 < len) n |= uint32_t(data[i + 2]);

            result += table[(n >> 18) & 0x3F];
            result += table[(n >> 12) & 0x3F];
            result += (i + 1 < len) ? table[(n >> 6) & 0x3F] : '=';
            result += (i + 2 < len) ? table[n & 0x3F] : '=';
        }

        return result;
    }
};
