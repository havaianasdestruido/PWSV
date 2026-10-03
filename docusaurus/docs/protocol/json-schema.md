---
sidebar_position: 3
title: "JSON Message Schema"
description: "Complete field-by-field reference and TypeScript definitions for the Protocol v2 JSON payload."
---

# JSON Message Schema

Every message transmitted by PWSV is a structured UTF-8 JSON object formatted according to Protocol v2.

---

## Complete JSON Payload Example

```json
{
  "protocol": 2,
  "time_sec": 18.250,
  "time_samples": 804825,
  "ppq": 36.500,
  "bpm": 120.0,
  "bar": 10,
  "beat": 0.50,
  "time_sig": [4, 4],
  "playing": true,
  "recording": false,
  "looping": true,
  "notes": [
    {
      "note": 60,
      "vel": 105,
      "ch": 1
    },
    {
      "note": 67,
      "vel": 98,
      "ch": 1
    }
  ]
}
```

---

## Field Reference

| Key | Type | Description | Example |
|---|---|---|---|
| `protocol` | `integer` | Protocol version number (always `2`). | `2` |
| `time_sec` | `number` | Playhead position in seconds from track start (3 decimal places). | `18.250` |
| `time_samples` | `integer` | Playhead position in audio sample frames. | `804825` |
| `ppq` | `number` | Pulses Per Quarter Note (continuous musical time in quarter notes). | `36.500` |
| `bpm` | `number` | Current DAW tempo in Beats Per Minute (1 decimal place). | `120.0` |
| `bar` | `integer` | 1-based musical bar index calculated from time signature and bar start. | `10` |
| `beat` | `number` | Fractional beat offset within the current musical bar (2 decimal places). | `0.50` |
| `time_sig` | `[integer, integer]` | Array representing `[numerator, denominator]` of current time signature. | `[4, 4]` |
| `playing` | `boolean` | `true` if DAW transport playback is active. | `true` |
| `recording` | `boolean` | `true` if DAW transport recording is active. | `false` |
| `looping` | `boolean` | `true` if DAW transport loop / cycle region is active. | `true` |
| `notes` | `array` | List of currently held active MIDI note objects. Empty if none active. | `[...]` |
| `notes[].note` | `integer` | MIDI note number (`0` to `127`). | `60` (Middle C / C4) |
| `notes[].vel` | `integer` | MIDI velocity of the note event (`0` to `127`). | `105` |
| `notes[].ch` | `integer` | 1-based MIDI channel number (`1` to `16`). | `1` |

---

## TypeScript Interface

```typescript
export interface PWSVNoteEvent {
  note: number;    // MIDI note number 0-127
  vel: number;     // Velocity 0-127
  ch: number;      // MIDI channel 1-16
}

export interface PWSVTelemetryMessage {
  protocol: 2;
  time_sec: number;
  time_samples: number;
  ppq: number;
  bpm: number;
  bar: number;
  beat: number;
  time_sig: [number, number];
  playing: boolean;
  recording: boolean;
  looping: boolean;
  notes: PWSVNoteEvent[];
}
```
