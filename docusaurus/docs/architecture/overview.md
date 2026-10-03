---
sidebar_position: 1
title: "Architecture Overview"
description: "High-level architectural overview of PWSV's audio pipeline, threading model, and networking engine."
---

# Architecture Overview

PWSV is engineered to bridge low-latency real-time audio systems with modern asynchronous networking protocols. Audio DSP threads operate under stringent timing constraints where memory allocations, system calls, and blocking I/O are forbidden. PWSV separates the real-time audio processing loop from the networking loop through dedicated threads and lock-protected snapshots.

---

## High-Level System Diagram

```text
┌────────────────────────────────────────────────────────────────────────┐
│                        DAW Audio Process Thread                        │
│                                                                        │
│  [ Audio / MIDI In ] ──► processBlock() ──► [ Audio / MIDI Out ]       │
│                                │                                       │
│                                ▼                                       │
│                   Extract Transport Info & MIDI                        │
│                                │                                       │
│                                ▼                                       │
│                   Thread-Safe State Snapshot                           │
│                      (std::mutex lock)                                 │
└────────────────────────────────┬───────────────────────────────────────┘
                                 │
                                 ▼
┌────────────────────────────────────────────────────────────────────────┐
│                     WebSocket Server Thread                            │
│                 (juce::Thread / "PatoWS_Server")                       │
│                                                                        │
│  ┌──────────────────────────────────────────────────────────────────┐  │
│  │ Non-blocking Socket Polling Loop (select(), 1ms timeout)         │  │
│  │                                                                  │  │
│  │  1. Check for incoming connections (acceptNewClients)            │  │
│  │  2. Handle HTTP/1.1 Upgrade WebSocket Handshake (SHA1 / Base64)  │  │
│  │  3. Read incoming traffic / detect disconnections (readFromClients)│
│  │  4. Evaluate shouldSend() condition (Hz, Beats, Minutes)         │  │
│  │  5. Build JSON payload & send RFC 6455 Text Frames (sendToAll)   │  │
│  └──────────────────────────────────────────────────────────────────┘  │
└────────────────────────────────┬───────────────────────────────────────┘
                                 │
                        TCP / IP │ ws://127.0.0.1:8080
                                 ▼
                     Connected Clients (Up to 8)
```

---

## Core Components

| Component | File(s) | Responsibility |
|---|---|---|
| **`WebSocketProcessorBase`** | `WebSocketProcessorBase.h/.cpp` | Base audio processor inheriting from `juce::AudioProcessor`. Manages APVTS parameters, MIDI event processing, playhead polling, and lifecycle of the server thread. |
| **`EffectProcessor`** | `EffectProcessor.h/.cpp` | Audio effect specialization. Has audio inputs/outputs, accepts no MIDI, and passes audio through unaltered. |
| **`GeneratorProcessor`** | `GeneratorProcessor.h/.cpp` | Audio generator specialization. Declared as a synth with MIDI inputs, clears audio buffers, and captures MIDI notes. |
| **`WebSocketServer`** | `WebSocketServer.h/.cpp` | Independent `juce::Thread` hosting a multi-client RFC 6455 WebSocket server using BSD sockets / Winsock. |
| **`PluginEditor`** | `PluginEditor.h/.cpp` | Modern GUI inheriting from `juce::AudioProcessorEditor`. Handles parameter attachments and dynamic visual state updates via a 10 Hz timer. |
| **`PositionData`** | `PositionData.h` | Plain C++ structs representing DAW transport snapshots and active note arrays. |
| **`Sha1` & `Base64`** | `Sha1.h`, `Base64.h` | Pure C++ zero-dependency cryptographic utilities for calculating the `Sec-WebSocket-Accept` header during handshakes. |

---

## Threading Model & Concurrency

1. **Audio Real-Time Thread (`processBlock`)**:
   - Queries `juce::AudioPlayHead` for position, BPM, time signature, bar start, and transport flags.
   - Iterates through `juce::MidiBuffer` to maintain an internal list of active Note-On / Note-Off events.
   - Acquires `posLock_` for sub-microsecond copying of `PositionData` into `WebSocketServer`.
   - Never blocks on networking operations or socket I/O.

2. **WebSocket Networking Thread (`PatoWS_Server`)**:
   - Executes independently inside `WebSocketServer::run()`.
   - Uses non-blocking socket descriptors (`O_NONBLOCK` on POSIX, `FIONBIO` on Windows).
   - Polls file descriptors with `select()` using a 1ms timeout.
   - Serializes JSON payloads and constructs RFC 6455 unmasked text frames.
   - Broadcasts data to all connected clients (supporting up to 8 concurrent clients).

3. **Message Thread / UI Thread (`PluginEditor`)**:
   - Renders the graphical user interface.
   - Synchronizes controls with host automation via `juce::AudioProcessorValueTreeState::SliderAttachment` and `ComboBoxAttachment`.
   - Polling timer running at 10 Hz queries atomic server status indicators to update the UI without lock contention.
