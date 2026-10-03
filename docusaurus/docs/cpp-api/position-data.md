---
sidebar_position: 7
title: "PositionData & NoteEvent"
description: "C++ reference for PositionData and NoteEvent structures."
---

# `PositionData` & `NoteEvent`

Header: `Source/PositionData.h`

`PositionData.h` defines plain data structures for snapshotting DAW transport metrics and polyphonic MIDI note events.

---

## `struct NoteEvent`

Represents an individual active MIDI note event.

```cpp
struct NoteEvent {
    int note     = 0;  // MIDI Note Number (0-127)
    int velocity = 0;  // MIDI Velocity (0-127)
    int channel  = 0;  // MIDI Channel (1-16; 0 is initial default)
};
```

---

## `struct PositionData`

Captures the full real-time transport snapshot from `juce::AudioPlayHead`.

```cpp
struct PositionData {
    double  timeInSeconds  = 0.0;     // Current transport time in seconds
    int64_t timeInSamples  = 0;       // Current transport time in sample frames
    double  ppq            = 0.0;     // Pulses Per Quarter Note (continuous musical time)
    double  bpm            = 120.0;   // Tempo in BPM
    int     barNumber      = 1;       // 1-based musical bar number
    double  beatInBar      = 0.0;     // Fractional beat within current bar
    int     timeSigNum     = 4;       // Time signature numerator (e.g. 4)
    int     timeSigDen     = 4;       // Time signature denominator (e.g. 4)
    bool    isPlaying      = false;   // DAW playback status
    bool    isRecording    = false;   // DAW recording status
    bool    isLooping      = false;   // DAW loop region enabled

    static constexpr int MAX_NOTES = 128;
    int  activeNoteNumbers[MAX_NOTES]    = {};
    int  activeNoteVelocities[MAX_NOTES] = {};
    int  activeNoteChannels[MAX_NOTES]   = {};
    int  activeNoteCount                 = 0; // Total active notes (array capped at MAX_NOTES)
};
```

:::info Array Bounds Safety
`activeNoteCount` reports the total number of currently held MIDI notes in the DAW block. However, the fixed arrays (`activeNoteNumbers`, `activeNoteVelocities`, `activeNoteChannels`) store at most `PositionData::MAX_NOTES` (128) entries. Always iterate using `std::min(activeNoteCount, PositionData::MAX_NOTES)`.
:::
