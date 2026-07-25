#pragma once
#include <cstdint>
#include <cstring>

class Sha1 {
public:
    Sha1() { reset(); }

    void reset() {
        state_[0] = 0x67452301;
        state_[1] = 0xEFCDAB89;
        state_[2] = 0x98BADCFE;
        state_[3] = 0x10325476;
        state_[4] = 0xC3D2E1F0;
        totalLen_ = 0;
        bufLen_ = 0;
    }

    void update(const void* data, size_t len) {
        auto ptr = static_cast<const uint8_t*>(data);
        size_t remaining = len;

        if (bufLen_ > 0) {
            while (remaining > 0 && bufLen_ < 64) {
                buffer_[bufLen_++] = *ptr++;
                --remaining;
            }
            if (bufLen_ == 64) {
                transform();
                totalLen_ += 64;
                bufLen_ = 0;
            }
        }

        while (remaining >= 64) {
            std::memcpy(buffer_, ptr, 64);
            transform();
            totalLen_ += 64;
            ptr += 64;
            remaining -= 64;
        }

        while (remaining > 0) {
            buffer_[bufLen_++] = *ptr++;
            --remaining;
        }
    }

    void finalize(uint8_t hash[20]) {
        uint64_t totalBits = (totalLen_ + bufLen_) * 8;

        buffer_[bufLen_++] = 0x80;
        if (bufLen_ > 56) {
            while (bufLen_ < 64)
                buffer_[bufLen_++] = 0;
            transform();
            bufLen_ = 0;
        }
        while (bufLen_ < 56)
            buffer_[bufLen_++] = 0;

        for (int i = 7; i >= 0; --i)
            buffer_[bufLen_++] = static_cast<uint8_t>(totalBits >> (i * 8));

        transform();

        for (int i = 0; i < 5; ++i) {
            hash[i * 4]     = static_cast<uint8_t>(state_[i] >> 24);
            hash[i * 4 + 1] = static_cast<uint8_t>(state_[i] >> 16);
            hash[i * 4 + 2] = static_cast<uint8_t>(state_[i] >> 8);
            hash[i * 4 + 3] = static_cast<uint8_t>(state_[i]);
        }
    }

private:
    uint32_t state_[5];
    uint8_t  buffer_[64];
    size_t   bufLen_;
    uint64_t totalLen_;

    static uint32_t rotl(uint32_t v, int n) {
        return (v << n) | (v >> (32 - n));
    }

    void transform() {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i)
            w[i] = (uint32_t(buffer_[i * 4]) << 24)
                 | (uint32_t(buffer_[i * 4 + 1]) << 16)
                 | (uint32_t(buffer_[i * 4 + 2]) << 8)
                 |  uint32_t(buffer_[i * 4 + 3]);
        for (int i = 16; i < 80; ++i)
            w[i] = rotl(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

        uint32_t a = state_[0], b = state_[1], c = state_[2];
        uint32_t d = state_[3], e = state_[4];

        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if      (i < 20) { f = (b & c) | (~b & d);          k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d;                   k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else              { f = b ^ c ^ d;                   k = 0xCA62C1D6; }

            uint32_t temp = rotl(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rotl(b, 30); b = a; a = temp;
        }

        state_[0] += a; state_[1] += b; state_[2] += c;
        state_[3] += d; state_[4] += e;
    }
};
