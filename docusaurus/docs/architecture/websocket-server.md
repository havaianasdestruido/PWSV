---
sidebar_position: 3
title: "WebSocket Server Engine"
description: "In-depth analysis of the non-blocking RFC 6455 server, socket lifecycle, handshake, framing, and client pooling."
---

# WebSocket Server Engine

`WebSocketServer` is a custom, zero-dependency C++ implementation of the **RFC 6455 WebSocket Protocol** designed specifically for real-time plugin embedding.

---

## Key Design Principles

1. **Zero External Networking Dependencies**:
   - Uses standard POSIX BSD sockets on Linux and macOS (`sys/socket.h`, `netinet/in.h`, `arpa/inet.h`, `fcntl.h`).
   - Uses Winsock 2 on Windows (`winsock2.h`, `ws2tcpip.h`, linking `ws2_32.lib`).
2. **Dedicated Worker Thread**:
   - Inherits from `juce::Thread` ("PatoWS_Server").
   - Decoupled from the DAW's real-time audio thread and message thread.
3. **Non-Blocking Multi-Client Event Loop**:
   - All server and client sockets are configured as non-blocking (`setNonBlocking()`).
   - `select()` monitors socket readiness with a 1 ms tick timeout (`timeval tv{0, 1000}`).
4. **Connection Pooling**:
   - Accommodates up to 8 simultaneous client connections.
   - Sockets exceeding the cap or failing handshake are closed immediately.

---

## Server Polling Loop

The worker thread executes the following main loop:

```cpp
void WebSocketServer::run() {
    initSockets();

    listenSocket_ = static_cast<int>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (listenSocket_ == -1) { lastError_ = "socket() failed"; return; }

    int opt = 1;
    setsockopt(listenSocket_, SOL_SOCKET, SO_REUSEADDR,
#ifdef _WIN32
        (const char*)&opt,
#else
        &opt,
#endif
        sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(static_cast<uint16_t>(port_));

    if (bind(listenSocket_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        lastError_ = "bind() failed on port " + juce::String(port_);
        closesocket(listenSocket_); listenSocket_ = -1; return;
    }

    if (listen(listenSocket_, 4) == SOCKET_ERROR) {
        lastError_ = "listen() failed";
        closesocket(listenSocket_); listenSocket_ = -1; return;
    }

    setNonBlocking(listenSocket_);
    running_    = true;
    lastSendTime_ = std::chrono::steady_clock::now();

    while (!threadShouldExit()) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listenSocket_, &readSet);
        SOCKET maxFd = listenSocket_;

        for (auto s : clients_) {
            FD_SET(s, &readSet);
            if (s > maxFd) maxFd = s;
        }

        timeval tv{ 0, 1000 };
        select(
#ifdef _WIN32
            0,
#else
            static_cast<int>(maxFd + 1),
#endif
            &readSet, nullptr, nullptr, &tv);

        if (FD_ISSET(listenSocket_, &readSet))
            acceptNewClients();

        readFromClients();

        if (shouldSend()) {
            sendToAll(buildJson());
            afterSend();
        }
    }

    for (auto s : clients_) closesocket(s);
    clients_.clear();
    clientCount_ = 0;
    closesocket(listenSocket_); listenSocket_ = -1;
    running_ = false;
}
```

---

## Handshake Implementation

When a new TCP client connects, `WebSocketServer::performHandshake(int sock)` reads the incoming HTTP Upgrade request and computes the RFC 6455 challenge response:

1. **Locate Key Header**: Scans for `Sec-WebSocket-Key: `.
2. **Concatenate GUID**: Appends the RFC 6455 magic UUID string:
   `258EAFA5-E914-47DA-95CA-5AB5DC76B97E`
3. **SHA-1 Hash**: Computes the 20-byte SHA-1 digest using `Sha1`.
4. **Base64 Encode**: Encodes the 20-byte digest using `Base64::encode()`.
5. **Send Response**:
   ```http
   HTTP/1.1 101 Switching Protocols\r\n
   Upgrade: websocket\r\n
   Connection: Upgrade\r\n
   Sec-WebSocket-Accept: <Base64Hash>\r\n\r\n
   ```

---

## RFC 6455 Framing & Transmission

Server-to-client WebSocket messages are sent as unmasked text frames (`0x81`):

```cpp
void WebSocketServer::sendTextFrame(int sock, const juce::String& payload) {
    auto utf8 = payload.toRawUTF8();
    auto len = static_cast<size_t>(payload.getNumBytesAsUTF8());
    uint8_t header[10];
    int headerLen = 0;

    header[0] = 0x81; // FIN bit set (0x80) + Text Opcode (0x01)
    if (len < 126) {
        header[1] = static_cast<uint8_t>(len);
        headerLen = 2;
    } else if (len < 65536) {
        header[1] = 126;
        header[2] = static_cast<uint8_t>((len >> 8) & 0xFF);
        header[3] = static_cast<uint8_t>(len & 0xFF);
        headerLen = 4;
    } else {
        header[1] = 127;
        for (int i = 7; i >= 0; --i)
            header[2 + (7 - i)] = static_cast<uint8_t>((len >> (i * 8)) & 0xFF);
        headerLen = 10;
    }

    send(sock, (const char*)header, headerLen, 0);
    send(sock, utf8, static_cast<int>(len), 0);
}
```
