---
sidebar_position: 1
title: "Effect vs. Generator Plugin"
description: "Comparative overview of PatoWebSocketEffect and PatoWebSocketGenerator plugins."
---

# Effect vs. Generator Plugin

PWSV ships as two distinct plugin binaries tailored for different DAW routing scenarios.

---

## Comparison Matrix

| Feature | `PatoWebSocketEffect` | `PatoWebSocketGenerator` |
|---|---|---|
| **Plugin Code** | `PWse` | `PWge` |
| **CLAP ID** | `com.pato.websocket-effect` | `com.pato.websocket-generator` |
| **Plugin Type** | Audio Effect / Insert Plugin | Instrument / Synth / Generator Plugin |
| **Audio Input** | Stereo (2 channels) | None |
| **Audio Output** | Stereo (2 channels) | Stereo (2 channels - silent) |
| **Audio Processing** | Pass-through (`processAudio(buffer)` is a no-op) | Clears buffer (`buffer.clear()`) |
| **MIDI Input** | `NEEDS_MIDI_INPUT: FALSE` | `NEEDS_MIDI_INPUT: TRUE` |
| **Synth Flag** | `IS_SYNTH: FALSE` | `IS_SYNTH: TRUE` |
| **MIDI Ingestion** | No MIDI processing | Active note tracking (NoteOn / NoteOff) |
| **Target DAW Slot** | Master bus, audio tracks, effect sends | MIDI tracks, virtual instrument slots |

---

## `PatoWebSocketEffect`

Designed to be placed anywhere in your DAW's audio signal chain (such as the Master track, bus stems, or individual audio channels).

### Implementation Details
```cpp
class EffectProcessor : public WebSocketProcessorBase {
public:
    EffectProcessor() : WebSocketProcessorBase(false) {}

    const juce::String getName() const override { return "Patos WS Effect"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

protected:
    void processAudio(juce::AudioBuffer<float>&) override {
        // Pass-through: Audio flows through completely unchanged
    }
};
```

---

## `PatoWebSocketGenerator`

Designed to be placed as a virtual instrument or MIDI generator on instrument tracks, allowing it to capture live MIDI performance or piano roll clips.

### Implementation Details
```cpp
class GeneratorProcessor : public WebSocketProcessorBase {
public:
    GeneratorProcessor() : WebSocketProcessorBase(true) {}

    const juce::String getName() const override { return "Patos WS Generator"; }
    bool acceptsMidi()  const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

protected:
    void processAudio(juce::AudioBuffer<float>& buffer) override {
        // Silent generator: clears audio buffer so no sound is produced
        buffer.clear();
    }
};
```
