---
slug: /
sidebar_position: 1
title: "PWSV Documentation"
description: "Complete documentation for PWSV (Pato's WebSocket VST), a low-latency DAW telemetry and MIDI streaming plugin suite."
---

# PWSV Documentation

Welcome to the official technical documentation for **PWSV (Pato's WebSocket VST)**.

PWSV is a high-performance audio plugin suite built with **JUCE 8** and **CLAP** that embeds a native, lightweight, non-blocking **RFC 6455 WebSocket Server** directly inside your Digital Audio Workstation (DAW). It broadcasts real-time DAW transport information, tempo, playhead position, time signature, bar/beat metrics, and active MIDI note events to any connected WebSocket client.

```
┌────────────────────────────────────────────────────────┐
│             Digital Audio Workstation (DAW)            │
│  (Ableton Live, FL Studio, Reaper, Bitwig, Cubase...)   │
│                                                        │
│  ┌──────────────────────────────────────────────────┐  │
│  │     Pato's WebSocket VST (PWSV)                  │  │
│  │  ┌────────────────────┐  ┌────────────────────┐  │  │
│  │  │ Audio/MIDI Engine  │  │  APVTS Parameters  │  │  │
│  │  └─────────┬──────────┘  └─────────┬──────────┘  │  │
│  │            ▼                       ▼             │  │
│  │     ┌──────────────────────────────────────┐     │  │
│  │     │  WebSocket Server Thread (RFC 6455)  │     │  │
│  │     └──────────────────┬───────────────────┘     │  │
│  └────────────────────────┼─────────────────────────┘  │
└───────────────────────────┼────────────────────────────┘
                            │ ws://127.0.0.1:8080 (JSON Protocol v2)
         ┌──────────────────┼───────────────────┐
         ▼                  ▼                   ▼
   ┌───────────┐      ┌───────────┐       ┌───────────┐
   │ Web Apps  │      │  Python   │       │  Visuals  │
   │ (Browser) │      │  Clients  │       │(TouchDes) │
   └───────────┘      └───────────┘       └───────────┘
```

---

## Key Highlights

- **Dual Plugin Architectures**:
  - **Effect Plugin (`PatoWebSocketEffect`)**: Seamless audio pass-through for master/audio tracks without introducing latency.
  - **Generator Plugin (`PatoWebSocketGenerator`)**: Instrument/synth slot plugin for capturing polyphonic MIDI note events, velocity, and channel routing.
- **Multiple Distribution Formats**: Native **VST3**, modern **CLAP** (via `clap-juce-extensions`), and standalone binaries.
- **Ultra-Low Latency & Non-Blocking Sockets**: Custom zero-dependency RFC 6455 server using POSIX / Winsock non-blocking polling (`select()`) on a dedicated thread, guaranteeing zero interference with DAW audio buffers.
- **3 Flexible Transmission Modes**:
  - **Hz Mode**: Fixed frequency updates from 1 Hz up to 240 Hz.
  - **Beats Mode**: Musical grid synchronization (1/1, 1/2, 1/4, 1/8, 1/16, 1/32, and triplets: 3/4, 3/8, 3/16, 3/32).
  - **Minutes Mode**: Configurable events per minute (60 to 14,400 /min).
- **Protocol v2 JSON Telemetry**: Real-time broadcast of transport state (`playing`, `recording`, `looping`), time in seconds and samples, PPQ (Pulses Per Quarter), BPM, musical bar, beat within bar, time signature numerator/denominator, and active MIDI note array.
- **Built-in Cryptography**: Zero-dependency C++ implementations of **SHA-1** and **Base64** for standard RFC 6455 handshake calculation (`Sec-WebSocket-Accept`).

---

## Documentation Roadmap

<div className="card-grid">
  <a className="doc-card" href="/docs/getting-started/quickstart">
    <h3>🚀 Getting Started</h3>
    <p>Install the plugins, insert them into your DAW, and verify connectivity in under 2 minutes.</p>
  </a>

  <a className="doc-card" href="/docs/architecture/overview">
    <h3>🏗️ Core Architecture</h3>
    <p>Explore the JUCE audio processing pipeline, threading model, and non-blocking networking core.</p>
  </a>

  <a className="doc-card" href="/docs/protocol/overview">
    <h3>📡 WebSocket Protocol v2</h3>
    <p>Read the complete JSON schema specification, framing format, and timing calculations.</p>
  </a>

  <a className="doc-card" href="/docs/cpp-api/">
    <h3>💻 C++ API Reference</h3>
    <p>In-depth reference for `WebSocketServer`, `WebSocketProcessorBase`, `PositionData`, and cryptographic helpers.</p>
  </a>

  <a className="doc-card" href="/docs/client-sdks/overview">
    <h3>🛠️ Client SDKs & Samples</h3>
    <p>Code examples and sample clients in HTML5 JavaScript, Python CLI, and Terminal ANSI visualizers.</p>
  </a>

  <a className="doc-card" href="/docs/integrations/touchdesigner">
    <h3>🎨 Integrations & Tutorials</h3>
    <p>Connect your DAW to TouchDesigner, OBS Studio, Unreal Engine 5, Unity, and Max/MSP.</p>
  </a>
</div>
