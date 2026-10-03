# PWSV — Pato's WebSocket VST

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![JUCE 8](https://img.shields.io/badge/JUCE-8.0.6-orange.svg)](https://juce.com/)
[![CLAP](https://img.shields.io/badge/CLAP-Supported-green.svg)](https://cleveraudio.org/)
[![Docusaurus](https://img.shields.io/badge/Docs-Docusaurus-cyan.svg)](docs/)

High-performance, ultra-low latency WebSocket DAW telemetry and polyphonic MIDI streaming VST3 & CLAP audio plugin suite built with **JUCE 8**.

---

## 📖 Documentation

- **Primary Webpage (Jekyll)**: [Main Landing Page](index.html)
- **Full Codebase Documentation (Docusaurus)**: [https://havaianasdestruido.github.io/PWSV/docs/](docs/)
  - [Getting Started & Quickstart](docs/getting-started/quickstart/)
  - [Installation Guide (Windows / macOS / Linux)](docs/getting-started/installation/)
  - [DAW Configuration (Ableton, FL Studio, Reaper, Bitwig, etc.)](docs/getting-started/daw-setup/)
  - [Architecture & Threading Model](docs/architecture/overview/)
  - [WebSocket Protocol v2 Specification](docs/protocol/overview/)
  - [C++ API Reference](docs/cpp-api/)
  - [Client SDKs (HTML5 JS, Python Raw Sockets, ANSI Visualizer)](docs/client-sdks/overview/)
  - [Integrations (TouchDesigner, OBS Studio, Unreal Engine, Unity, Max/MSP)](docs/integrations/touchdesigner/)

---

## ⚡ Overview

PWSV embeds a native, lightweight, non-blocking **RFC 6455 WebSocket Server** directly into audio plugin formats (VST3, CLAP, and Standalone). It streams real-time transport states, tempo, playhead position, time signature, bar/beat metrics, and active MIDI note events to connected visualizers, browsers, game engines, and live stage controllers.

```
┌────────────────────────────────────────────────────────┐
│             Digital Audio Workstation (DAW)            │
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
                            │ ws://127.0.0.1:8080 (Protocol v2)
         ┌──────────────────┼───────────────────┐
         ▼                  ▼                   ▼
   ┌───────────┐      ┌───────────┐       ┌───────────┐
   │ Web Apps  │      │  Python   │       │Visualizers│
   │ (Browser) │      │  Clients  │       │(TouchDes) │
   └───────────┘      └───────────┘       └───────────┘
```

---

## 🔌 Dual Plugin Architecture

| Plugin | Type | Routing | MIDI Ingestion | Audio Processing |
|---|---|---|---|---|
| **`PatoWebSocketEffect`** | Audio Effect | Master & Audio Insert Tracks | None | Pass-through (Zero latency) |
| **`PatoWebSocketGenerator`** | Virtual Instrument | MIDI & Synth Tracks | Polyphonic NoteOn/NoteOff | Silent buffer |

---

## 📡 Protocol v2 Payload Example

```json
{
  "protocol": 2,
  "time_sec": 14.523,
  "time_samples": 640464,
  "ppq": 29.046,
  "bpm": 120.0,
  "bar": 8,
  "beat": 1.05,
  "time_sig": [4, 4],
  "playing": true,
  "recording": false,
  "looping": false,
  "notes": [
    { "note": 60, "vel": 100, "ch": 1 },
    { "note": 64, "vel": 92, "ch": 1 }
  ]
}
```

---

## 🚀 Running the Documentation Locally

To build and preview both the **Jekyll primary website** (`/`) and the **Docusaurus documentation** (`/docs`):

```bash
# 1. Install Docusaurus dependencies
npm --prefix docusaurus install

# 2. Build Docusaurus documentation and start unified preview server
npm start
```

Visit:
- Primary Homepage: `http://localhost:3000/`
- Docusaurus Documentation: `http://localhost:3000/docs/`

---

## 🔨 Building the Plugins from Source

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

---

## 📄 License

This project is open-source under the [MIT License](LICENSE).
