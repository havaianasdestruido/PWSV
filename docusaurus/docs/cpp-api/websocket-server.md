---
sidebar_position: 2
title: "WebSocketServer"
description: "In-depth C++ API reference for WebSocketServer class."
---

# `WebSocketServer`

Inherits from: `juce::Thread`  
Header: `Source/WebSocketServer.h`  
Source: `Source/WebSocketServer.cpp`

`WebSocketServer` manages a non-blocking TCP socket server conforming to RFC 6455. It runs inside an independent worker thread named `"PatoWS_Server"`.

---

## Public Methods

### `WebSocketServer()`
Default constructor. Initializes the underlying `juce::Thread` with the name `"PatoWS_Server"`.

### `~WebSocketServer() override`
Destructor. Invokes `stop()` to guarantee that worker threads and socket descriptors are cleaned up before destruction.

### `bool start(int port)`
Stops any existing server instance, sets `port_ = port`, and spawns the background worker thread via `startThread(Thread::Priority::normal)`.
- **Parameters**: `port` — TCP port to bind (e.g. `8080`).
- **Returns**: `true` upon successful thread initiation.

### `void stop()`
Signals the thread to exit via `signalThreadShouldExit()`, waits up to 2,000 ms (`stopThread(2000)`), closes all active client sockets, closes the listen socket, and resets client count.

### `void updatePosition(const PositionData& data)`
Thread-safely copies the latest `PositionData` snapshot into internal storage using a `std::lock_guard<std::mutex>` on `posLock_`.
- **Parameters**: `data` — Latest transport and MIDI snapshot from the audio thread.

### `void setMode(int mode)`
Atomically sets the active transmission mode (`0`: Hz, `1`: Beats, `2`: Minutes).

### `void setRate(float hz)`
Atomically sets the broadcast frequency in Hz (`rate_`).

### `void setBeatDivision(int index)`
Atomically sets the index for musical PPQ beat division (`beatDiv_`, range 0–9).

### `void setBpm(double bpm)`
Atomically updates current BPM (`bpm_`).

### `int getConnectedClientCount() const`
Returns the current number of actively connected WebSocket clients.

### `bool isRunning() const`
Returns `true` if the server is active and listening for incoming connections.

### `juce::String getLastError() const`
Returns any error string recorded during socket initialization or binding.

---

## Constants & Static Members

```cpp
static constexpr int NUM_BEAT_DIVISIONS = 10;
static const char*   beatDivisionLabels[NUM_BEAT_DIVISIONS];
static const double  beatDivisionPpq[NUM_BEAT_DIVISIONS];
```

---

## Private Methods

- `void run() override`: Main event loop with `select()` and 1ms timeout.
- `void acceptNewClients()`: Accepts pending TCP connections (up to max 8) and executes `performHandshake()`.
- `bool performHandshake(int sock)`: Validates `Sec-WebSocket-Key`, computes SHA-1/Base64 challenge response, and sends HTTP 101 Switching Protocols.
- `void readFromClients()`: Drains incoming client buffers and detects connection dropouts.
- `void disconnectClient(size_t index)`: Closes socket descriptor and erases client from active list.
- `void sendTextFrame(int sock, const juce::String& payload)`: Constructs RFC 6455 unmasked text frames.
- `void sendToAll(const juce::String& payload)`: Dispatches frame to all active clients.
- `bool shouldSend()`: Evaluates timing conditions based on active mode.
- `void afterSend()`: Records `lastSentPpq_` and `lastSendTime_`.
- `juce::String buildJson()`: Serializes `PositionData` to Protocol v2 JSON string.
- `PositionData getPosition() const`: Thread-safe reader for `pos_`.
