---
sidebar_position: 10
title: "FAQ & Troubleshooting"
description: "Frequently asked questions, common error messages, and troubleshooting tips."
---

# FAQ & Troubleshooting

Find answers to common questions and solutions to issues encountered when configuring or connecting to PWSV.

---

## Frequently Asked Questions

### Does PWSV introduce audio latency?
**No.** `PatoWebSocketEffect` performs zero DSP processing inside `processAudio` (it is a pass-through). All networking and JSON serialization occur on a separate background thread (`PatoWS_Server`). Audio processing blocks return immediately without waiting for network I/O.

### Can I run multiple plugin instances?
**Yes.** However, each instance must listen on a distinct TCP port. Adjust the **Port** slider on subsequent instances (e.g. `8080`, `8081`, `8082`).

### Why do I see 0 connected clients?
Ensure that your client application has successfully completed the RFC 6455 handshake. If connecting from a browser, verify that your client connects to `ws://localhost:<port>` or `ws://127.0.0.1:<port>`.

### Does PWSV support WSS (Encrypted WebSockets / TLS)?
PWSV currently implements plain `ws://` to maintain zero external cryptographic library dependencies. If you need secure `wss://` over the internet or across networks, place a reverse proxy (such as Nginx, Caddy, or Cloudflare Tunnel) in front of the local port.

---

## Troubleshooting Guide

### 1. `bind() failed on port 8080` (Status label turns Red)
- **Cause**: Another application or a previously crashed DAW instance is already holding port 8080.
- **Fix**:
  - Change the **Port** slider in the plugin to another value (e.g. `8088` or `9000`).
  - Or terminate the conflicting process holding port 8080 using `netstat -ano | findstr 8080` on Windows or `lsof -i :8080` on macOS/Linux.

### 2. Client receives `Handshake failed` or connection resets
- **Cause**: The client is not supplying standard RFC 6455 headers (`Upgrade: websocket`, `Connection: Upgrade`, `Sec-WebSocket-Key`).
- **Fix**: Use standard WebSocket clients or review the [RFC 6455 Handshake Guide](./protocol/framing-and-handshake.md).

### 3. Generator Plugin produces no MIDI notes in the `"notes"` array
- **Cause**: The track has not routed MIDI events into the plugin.
- **Fix**: In your DAW, ensure the track input is set to your MIDI Controller, **Record Arm** is active, and track monitoring is turned ON.

### 4. Windows Defender / Firewall Prompt
- **Cause**: The first time a DAW opens a listening TCP socket, Windows Firewall prompts for permission.
- **Fix**: Allow local network permissions for your DAW executable.
