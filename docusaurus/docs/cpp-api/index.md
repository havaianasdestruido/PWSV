---
sidebar_position: 1
title: "C++ API Overview"
description: "Architecture and index of all C++ classes, structures, and utility headers in PWSV."
---

# C++ API Overview

The PWSV codebase is designed in standard modern **C++17** using the **JUCE 8** framework. It is structured into audio processing, GUI presentation, networking, and cryptographic components.

---

## Class & Header Index

```text
Source/
├── PositionData.h              - Struct definitions for NoteEvent and PositionData
├── Sha1.h                      - Zero-dependency SHA-1 cryptographic hasher
├── Base64.h                    - Zero-dependency Base64 encoder
├── WebSocketServer.h/.cpp      - Threaded RFC 6455 WebSocket Server implementation
├── WebSocketProcessorBase.h/.cpp- Abstract base audio processor (APVTS & playhead)
├── EffectProcessor.h/.cpp      - Audio effect processor specialization (Pass-through)
├── GeneratorProcessor.h/.cpp   - Audio generator/synth specialization (MIDI capture)
└── PluginEditor.h/.cpp         - JUCE GUI editor and dynamic UI controllers
```

---

## Detailed Class Pages

- [`WebSocketServer`](./websocket-server.md): Threaded RFC 6455 socket server, connection management, framing, and JSON serialization.
- [`WebSocketProcessorBase`](./websocket-processor-base.md): Base `juce::AudioProcessor` managing APVTS parameters, MIDI parsing, and transport state extraction.
- [`EffectProcessor`](./effect-processor.md): Audio effect specialization for master and audio insert channels.
- [`GeneratorProcessor`](./generator-processor.md): Synth specialization for MIDI note ingestion.
- [`PluginEditor`](./plugin-editor.md): JUCE GUI editor with dynamic layout and 10 Hz telemetry polling.
- [`PositionData`](./position-data.md): Data structures for DAW transport parameters and active MIDI note storage.
- [`Cryptography (Sha1 & Base64)`](./cryptography.md): Cryptographic algorithms powering the WebSocket opening handshake.
