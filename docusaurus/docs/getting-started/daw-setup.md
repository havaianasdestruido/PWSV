---
sidebar_position: 3
title: "DAW Setup Guide"
description: "Configuration and usage instructions across major DAWs including Ableton Live, FL Studio, Reaper, Bitwig, Cubase, and Logic Pro."
---

# DAW Setup Guide

This guide details how to configure PWSV within popular Digital Audio Workstations to capture transport telemetry and MIDI note data.

---

## 1. Ableton Live

### Transport Telemetry (Master Track)
1. In Ableton Live, navigate to your **Master Track**.
2. From the browser, search for `Patos WebSocket VST Effect` (under Plug-Ins → VST3 or CLAP).
3. Drag the plugin onto the Master track.
4. The plugin operates in pass-through mode and will not alter audio. Transport data (`playing`, `bpm`, `ppq`, `bar`, `beat`) will stream automatically.

### MIDI Capture (Instrument / MIDI Track)
1. Create a new MIDI Track.
2. Insert `Patos WebSocket VST Generator`.
3. Set the track's **MIDI From** to your MIDI Keyboard or another track's MIDI output.
4. Arm the track for recording. When you play keys, notes will be broadcast via WebSocket inside the `"notes"` array.

---

## 2. FL Studio

### Master / Mixer Insert (Effect)
1. Open the FL Studio **Mixer** (F9).
2. Select the **Master** insert or an auxiliary insert.
3. In an FX slot, select `Patos WebSocket VST Effect`.
4. The plugin binds to default port 8080 during audio initialization (`WebSocketProcessorBase::prepareToPlay`).

### Channel Rack (Generator)
1. In the **Channel Rack**, click `+` and select `More plugins...`.
2. Find `Patos WebSocket VST Generator` and add it as an instrument channel.
3. Draw notes in the Piano Roll or play from a MIDI controller. FL Studio will pass NoteOn/NoteOff events directly to the plugin.

---

## 3. Cockos Reaper

1. Create a new track.
2. Click the **FX** button on the track.
3. Filter by `VST3: Patos WebSocket VST Effect` or `CLAP: Patos WebSocket VST Effect`.
4. If using `Patos WebSocket VST Generator`, set the track input to **MIDI: All Channels** and enable **Record Arm** and **Record Monitoring: ON**.

---

## 4. Bitwig Studio

Bitwig natively supports both **VST3** and **CLAP**:
1. Open the Bitwig device browser.
2. Select **CLAP** category and insert `Patos WebSocket VST Effect` or `Patos WebSocket VST Generator`.
3. In Bitwig's sandboxed plugin environment, network socket access is fully functional.

---

## 5. Steinberg Cubase / Nuendo

1. Insert `Patos WebSocket VST Effect` on your Stereo Out / Master bus.
2. For MIDI generator usage, create an Instrument Track with `Patos WebSocket VST Generator`.
3. Ensure Cubase's playhead position broadcast is active during transport playback.

---

## Multiple Plugin Instances & Port Configuration

:::warning Port Conflicts
Only one process or plugin instance can bind to a given TCP port at any time.
:::

If you run multiple instances of PWSV (for example, one Effect on the Master track and one Generator on a synth track):
- Change the **Port** slider on the second instance (e.g., from `8080` to `8081`).
- The status label will reflect the new active listening port.
- Your external clients can connect to `ws://127.0.0.1:8080` for master transport and `ws://127.0.0.1:8081` for synth notes.
