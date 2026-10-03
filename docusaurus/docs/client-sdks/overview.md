---
sidebar_position: 1
title: "Client SDKs & Samples Overview"
description: "Overview of the sample client implementations provided with PWSV."
---

# Client SDKs & Samples Overview

PWSV includes several complete sample implementations in the `sample/` directory demonstrating how to connect to the plugin from web browsers, CLI scripts, and terminal visualizers.

---

## Included Samples

| File | Language / Environment | Description |
|---|---|---|
| [`sample/monitor.html`](./javascript-web.md) | HTML5 / JavaScript | Web browser client displaying real-time DAW transport state and active musical notes. |
| [`sample/test_ws.py`](./python.md) | Python 3 (Raw Sockets) | Lightweight Python script implementing RFC 6455 framing and printing real-time transport stats. |
| [`sample/monitor_cli.py`](./python.md) | Python 3 (Raw Sockets) | Clean single-line transport monitoring CLI with automatic carriage return updates. |
| [`sample/keys.py`](./python.md) | Python 3 (Raw Sockets) | Real-time MIDI key name translator converting note numbers to chord / key strings (e.g. `['C4', 'E4', 'G4']`). |
| [`sample/visualizer.py`](./terminal-visualizer.md) | Python 3 (ANSI Terminal) | Rich interactive terminal visualizer with dynamic beat grid, tempo tracker, spectrum analyzer, and timeline. |

---

## Connection Quick Checklist

1. Ensure the PWSV plugin is loaded in your DAW and running.
2. Note the TCP port configured in the plugin UI (default: `8080`).
3. Connect using standard WebSocket protocols to `ws://127.0.0.1:8080`.
