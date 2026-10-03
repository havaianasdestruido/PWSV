---
sidebar_position: 2
title: "Parameter Reference"
description: "Complete reference for all APVTS audio parameters, automation ids, ranges, and defaults."
---

# Parameter Reference

All plugin parameters are managed through JUCE's `AudioProcessorValueTreeState` (APVTS), enabling full DAW automation, preset saving, and state recall.

---

## Parameter Table

| Parameter ID | Name | Type | Range / Choices | Default | Description |
|---|---|---|---|---|---|
| `mode` | **Mode** | Choice | `0`: Hz<br/>`1`: Beats<br/>`2`: Minutes | `0` (Hz) | Determines the timing engine used for sending WebSocket telemetry packets. |
| `rate` | **Rate** | Float | `1.0` to `240.0` (step `0.1`) | `10.0 Hz` | Target broadcast frequency in Hz (Mode 0) or normalized rate (Mode 2). |
| `beatDiv` | **Beat Division** | Choice | 10 musical divisions (see table below) | `2` (`1/4`) | Musical grid interval for triggering WebSocket broadcasts in Beats mode. |
| `port` | **Port** | Float | `1024.0` to `65535.0` (step `1.0`) | `8080.0` | TCP port on which the internal WebSocket server listens. |

---

## Detailed Parameter Breakdown

### 1. `mode` (Mode)
- **Parameter ID**: `mode`
- **Internal Values**:
  - `0`: **Hz** — Broadcast packets at a fixed time interval computed as `1.0 / rate`.
  - `1`: **Beats** — Broadcast packets synchronized to DAW musical transport grid divisions.
  - `2`: **Minutes** — Broadcast packets at events-per-minute rate (`rate = val / 60.0`).

### 2. `rate` (Rate)
- **Parameter ID**: `rate`
- **Default**: `10.0 Hz`
- **Range**: `1.0` to `240.0`
- **Behavior**:
  - In **Hz Mode**, sets update frequency directly between 1 Hz (1 update/sec) and 240 Hz (240 updates/sec).
  - In **Minutes Mode**, the UI scales this slider from 60 to 14,400 events/minute, mapping to 1.0–240.0 Hz internally.

### 3. `beatDiv` (Beat Division)
- **Parameter ID**: `beatDiv`
- **Available Options**:

| Index | Label | Musical Value | PPQ Interval |
|---|---|---|---|
| `0` | `1/1` | Whole Note | 4.0 PPQ |
| `1` | `1/2` | Half Note | 2.0 PPQ |
| `2` | `1/4` | Quarter Note (Beat) | 1.0 PPQ |
| `3` | `1/8` | Eighth Note | 0.5 PPQ |
| `4` | `1/16` | Sixteenth Note | 0.25 PPQ |
| `5` | `1/32` | Thirty-second Note | 0.125 PPQ |
| `6` | `3/4` | Quarter-note Triplet | 2/3 PPQ (0.6667) |
| `7` | `3/8` | Eighth-note Triplet | 1/3 PPQ (0.3333) |
| `8` | `3/16` | Sixteenth-note Triplet | 0.5/3 PPQ (0.1667) |
| `9` | `3/32` | Thirty-second-note Triplet | 0.25/3 PPQ (0.0833) |

### 4. `port` (TCP Port)
- **Parameter ID**: `port`
- **Default**: `8080`
- **Range**: `1024` to `65535`
- **Dynamic Rebinding**: When this parameter changes, the processor stops the running server and immediately rebinds the listen socket to the new port without requiring a DAW restart.
