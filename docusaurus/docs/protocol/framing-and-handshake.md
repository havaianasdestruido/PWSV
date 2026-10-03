---
sidebar_position: 2
title: "RFC 6455 Handshake & Framing"
description: "Technical specification of the RFC 6455 opening handshake, SHA-1 / Base64 calculation, and WebSocket frame construction."
---

# RFC 6455 Handshake & Framing

PWSV implements the standard RFC 6455 WebSocket opening handshake and text framing protocol without requiring external third-party libraries.

---

## Opening Handshake Specification

### 1. Client HTTP Request
Clients initiate the connection by sending an HTTP Upgrade request:

```http
GET / HTTP/1.1
Host: 127.0.0.1:8080
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
Sec-WebSocket-Version: 13
```

### 2. Server Key Verification Algorithm
To formulate the `Sec-WebSocket-Accept` header, PWSV executes the following cryptographic steps:

1. Extract the string value of `Sec-WebSocket-Key:` (e.g. `dGhlIHNhbXBsZSBub25jZQ==`).
2. Concatenate the standard RFC 6455 GUID `258EAFA5-E914-47DA-95CA-5AB5DC76B97E`:
   ```text
   "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-5AB5DC76B97E"
   ```
3. Compute the 160-bit (20-byte) SHA-1 hash of this concatenated UTF-8 string using the internal `Sha1` class.
4. Base64-encode the resulting 20-byte binary digest using `Base64::encode()`:
   ```text
   "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="
   ```

### 3. Server HTTP 101 Response
```http
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=

```

---

## Frame Structure

PWSV transmits telemetry data as **Unmasked Text Frames** (Opcode `0x1`).

```text
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                     Payload Data (JSON UTF-8)                 |
+---------------------------------------------------------------+
```

### Header Byte Encoding in PWSV
- **Byte 0**: `0x81` (`FIN=1`, `RSV=0`, `Opcode=1` for Text Frame).
- **Byte 1–9 (Payload Length)**:
  - If length &lt; 126: Byte 1 is `static_cast<uint8_t>(len)` (Header length = 2 bytes).
  - If 126 &le; length &lt; 65536: Byte 1 is `126`, Bytes 2–3 hold the 16-bit big-endian length (Header length = 4 bytes).
  - If length &ge; 65536: Byte 1 is `127`, Bytes 2–9 hold the 64-bit big-endian length (Header length = 10 bytes).
