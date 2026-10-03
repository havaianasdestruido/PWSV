---
sidebar_position: 1
title: "Quick Start"
description: "Get up and running with PWSV in less than two minutes."
---

# Quick Start Guide

This guide will walk you through launching PWSV in your DAW and receiving your first live telemetry broadcast via WebSocket.

---

## 1. Install or Build PWSV

If you have pre-compiled binaries, run the Windows installer `installer/setup.bat` as Administrator, or copy the plugin files into your system VST3/CLAP folder:

- **Windows VST3**: `C:\Program Files\Common Files\VST3\Patos WebSocket VST Effect.vst3`
- **Windows CLAP**: `C:\Program Files\Common Files\CLAP\Patos WebSocket VST Effect.clap`

*(For full installation instructions across Windows, macOS, and Linux, see the [Installation Guide](./installation.md).)*

---

## 2. Load the Plugin in your DAW

1. Open your DAW (e.g. Ableton Live, FL Studio, Reaper, Bitwig, Cubase).
2. Rescan your plugin library.
3. Insert **Patos WebSocket VST Effect** onto your Master track (or any audio track).
4. If you wish to capture MIDI notes from an instrument track, insert **Patos WebSocket VST Generator** instead.
5. Open the plugin window. You should see the dark-themed UI:

```text
┌──────────────────────────────────────────────┐
│             Pato's WebSocket VST             │
│                                              │
│  Mode:     [ Hz ▾ ]                          │
│  Rate:     [ 10.0 Hz ─────────●─── ]  10.0   │
│  Port:     [ 8080 ────────────●─── ]  8080   │
│                                              │
│  Server running on port 8080 - 0 clients     │
│  MIDI: 0 | Active: 0                  v1.1.1 │
└──────────────────────────────────────────────┘
```

By default, the server binds to all network interfaces (`INADDR_ANY` / `0.0.0.0`) on port `8080`, allowing local and LAN clients to receive transport data and active MIDI notes. If remote access is not desired, standard firewall rules can restrict traffic to the loopback interface (`127.0.0.1`).

---

## 3. Test Connectivity in your Browser

Open `sample/monitor.html` in any modern web browser (Google Chrome, Firefox, Safari, Edge).

1. Ensure the port is set to `8080`.
2. Click **Connect**.
3. The status label will change to `Connected`, and you will see incoming JSON packets updating in real time:

```json
{
  "protocol": 2,
  "time_sec": 4.125,
  "time_samples": 181912,
  "ppq": 8.250,
  "bpm": 120.0,
  "bar": 3,
  "beat": 0.25,
  "time_sig": [4, 4],
  "playing": true,
  "recording": false,
  "looping": false,
  "notes": []
}
```

---

## 4. Run the Python Visualizer

To see an interactive ASCII / ANSI color visualizer in your terminal:

```bash
cd sample
python visualizer.py 8080
```

When you hit **Play** in your DAW, the terminal visualizer will render an animated beat grid, tempo tracker, spectrum analyzer, and timeline synced to your DAW transport clock!
