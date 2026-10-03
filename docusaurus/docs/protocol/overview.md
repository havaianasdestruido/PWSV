---
sidebar_position: 1
title: "Protocol v2 Overview"
description: "Overview of the PWSV WebSocket Protocol version 2 specification."
---

# WebSocket Protocol v2 Overview

PWSV uses **Protocol Version 2**, a lightweight, JSON-over-WebSocket protocol compliant with [RFC 6455](https://datatracker.ietf.org/doc/html/rfc6455).

---

## Protocol Characteristics

- **Transport**: Standard WebSocket over TCP (`ws://<host>:<port>`).
- **Default Endpoint**: `ws://127.0.0.1:8080` (or configured port).
- **Format**: UTF-8 encoded JSON text frames (`Opcode 0x1`).
- **Communication Direction**: Unidirectional server-to-client telemetry broadcast (server accepts incoming connections and streams state).
- **Concurrency**: Up to 8 concurrent connected clients per plugin instance.

---

## Message Workflow

```text
Client                                                  PWSV Server
  │                                                          │
  │─── HTTP GET / (Upgrade: websocket, Sec-WebSocket-Key) ──►│
  │                                                          │
  │◄── HTTP 101 Switching Protocols (Sec-WebSocket-Accept) ──│
  │                                                          │
  │                     [ Connection Open ]                  │
  │                                                          │
  │◄── Text Frame: JSON Transport Telemetry Packet ──────────│
  │◄── Text Frame: JSON Transport Telemetry Packet ──────────│
  │◄── Text Frame: JSON Transport Telemetry Packet ──────────│
  │                                                          │
  │─── TCP Close / Disconnect ──────────────────────────────►│
  │                     [ Connection Closed ]                │
```

---

## Example Message Payload

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
    {
      "note": 60,
      "vel": 100,
      "ch": 1
    },
    {
      "note": 64,
      "vel": 92,
      "ch": 1
    }
  ]
}
```

---

## Differences from Protocol v1

- Added explicit `"protocol": 2` identifier to payloads.
- Added structured `"notes"` array containing active MIDI notes with velocity and 1-indexed MIDI channel.
- Added musical context: `bar` (bar number) and `beat` (fractional beat position within current bar).
- Added transport flags: `recording` and `looping`.
