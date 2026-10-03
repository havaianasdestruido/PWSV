---
sidebar_position: 8
title: "Cryptography (Sha1 & Base64)"
description: "C++ API reference for zero-dependency Sha1 hasher and Base64 encoder."
---

# Cryptography (`Sha1` & `Base64`)

Headers: `Source/Sha1.h`, `Source/Base64.h`

PWSV provides standalone, zero-dependency C++ implementations of SHA-1 hashing and Base64 encoding to fulfill the RFC 6455 opening handshake requirement without pulling in OpenSSL or external libraries.

---

## `class Sha1`

Implements the **FIPS PUB 180-1 / RFC 3174 Secure Hash Algorithm 1**.

### Class Declaration
```cpp
class Sha1 {
public:
    Sha1();
    void reset();
    void update(const void* data, size_t len);
    void finalize(uint8_t hash[20]);
};
```

### Usage Example
```cpp
#include "Sha1.h"
#include <string>
#include <cstdint>

std::string input = "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-5AB5DC76B97E";
Sha1 hasher;
hasher.update(input.data(), input.size());
uint8_t digest[20];
hasher.finalize(digest);
// Expected 20-byte SHA-1 digest for RFC 6455 test key:
// {0xb3, 0x7a, 0x4f, 0x2c, 0xc0, 0x62, 0x4f, 0x16, 0x90, 0xf6,
//  0x46, 0x06, 0xcf, 0x38, 0x59, 0x45, 0xb2, 0xbe, 0xc4, 0xea}
```

---

## `class Base64`

Implements **RFC 4648 Base64 Encoding**.

### Class Declaration
```cpp
class Base64 {
public:
    static std::string encode(const uint8_t* data, size_t len);
};
```

### Usage Example
```cpp
#include "Base64.h"

// Encodes the 20-byte SHA-1 digest calculated above
std::string base64String = Base64::encode(digest, 20);
// Result: "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="
```
