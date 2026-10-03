---
sidebar_position: 4
title: "Timing & Transmission Modes"
description: "Detailed mathematical and logical analysis of the three transmission modes: Hz, Beats, and Minutes."
---

# Timing & Transmission Modes

PWSV provides three distinct timing engines in `WebSocketServer::shouldSend()` to determine when telemetry frames are broadcast.

---

## Mode 0: Hz (Time Frequency Mode)

In **Hz Mode**, telemetry frames are dispatched at fixed real-time clock intervals.

### Logic & Formula
The elapsed time since the previous transmission is measured using `std::chrono::steady_clock`:

```text
elapsed_seconds = current_time - last_send_time
```

A frame is sent whenever:

```text
elapsed_seconds >= (1.0 / rate_hz)
```

- **Range**: `1.0 Hz` (1 message per second) to `240.0 Hz` (240 messages per second).
- **Use Case**: Continuous UI telemetry, high-framerate visualizers (e.g. 60 Hz or 120 Hz displays), external hardware monitors.

---

## Mode 1: Beats (Musical Quantization Mode)

In **Beats Mode**, transmissions are strictly locked to the DAW's musical timeline based on the selected **Beat Division** (`div_ppq`).

### Logic & Formula
The current PPQ (Pulses Per Quarter Note) position from the DAW playhead is evaluated:

```text
current_grid = floor(current_ppq / div_ppq)
last_grid    = floor(last_sent_ppq / div_ppq)
```

A frame is dispatched if:
1. Transport playback just started (`isPlaying && !wasPlaying`), OR
2. `current_grid != last_grid` (the playhead has crossed into a new musical subdivision).

If the DAW is stopped (`!isPlaying`), no frames are sent in Beats mode.

:::info Loop & Seek Behavior
`lastSentPpq_` is updated exclusively inside `afterSend()` upon packet transmission. When transport loops or seeks to a new timeline position, the next audio block calculates a new `current_grid`. If this differs from `last_grid`, a telemetry frame is dispatched immediately on the first block after the jump.
:::

### Beat Division Reference Table

| Index | Sub-division | Musical Name | PPQ Value (`div_ppq`) |
|---|---|---|---|
| `0` | `1/1` | Whole Note | `4.0` |
| `1` | `1/2` | Half Note | `2.0` |
| `2` | `1/4` | Quarter Note (Beat) | `1.0` |
| `3` | `1/8` | Eighth Note | `0.5` |
| `4` | `1/16` | Sixteenth Note | `0.25` |
| `5` | `1/32` | Thirty-second Note | `0.125` |
| `6` | `3/4` | Quarter-note Triplet | `2.0 / 3.0 ≈ 0.666667` |
| `7` | `3/8` | Eighth-note Triplet | `1.0 / 3.0 ≈ 0.333333` |
| `8` | `3/16` | Sixteenth-note Triplet | `0.5 / 3.0 ≈ 0.166667` |
| `9` | `3/32` | Thirty-second-note Triplet | `0.25 / 3.0 ≈ 0.083333` |

- **Use Case**: Step sequencers, rhythm-locked lighting controllers (DMX), BPM-quantized animation cues.

---

## Mode 2: Minutes (Events Per Minute)

In **Minutes Mode**, the broadcast rate is specified as the total number of events desired per minute (`events_per_minute`).

### Conversion Formula
The frequency in Hz is derived as:

```text
rate_hz = events_per_minute / 60.0
```

The elapsed time condition follows the Hz calculation with `rate_hz`.

- **Range**: `60.0 /min` (1.0 Hz) to `14400.0 /min` (240.0 Hz).
- **Use Case**: Logging systems, analytics, or tempo-independent pulse timing.
