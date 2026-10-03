---
sidebar_position: 3
title: "WebSocketProcessorBase"
description: "In-depth C++ API reference for WebSocketProcessorBase class."
---

# `WebSocketProcessorBase`

Inherits from: `juce::AudioProcessor`  
Header: `Source/WebSocketProcessorBase.h`  
Source: `Source/WebSocketProcessorBase.cpp`

`WebSocketProcessorBase` is the abstract base class for PWSV audio plugins. It encapsulates parameter state management (`AudioProcessorValueTreeState`), playhead transport polling, MIDI note accumulation, and lifecycle orchestration for `WebSocketServer`.

---

## Constructor & Destructor

### `explicit WebSocketProcessorBase(bool isSynth)`
- **Parameters**: `isSynth` — When `true`, configures the processor with output buses only. When `false`, configures stereo input and stereo output buses.
- **Behavior**: Initializes `apvts_` with the layout returned by `createParameterLayout()` and instantiates `server_`.

### `~WebSocketProcessorBase() override`
Stops the embedded `WebSocketServer` instance.

---

## Overridden JUCE Methods

### `void prepareToPlay(double sampleRate, int samplesPerBlock) override`
Saves current `sampleRate_`, reads `"port"` from APVTS, and starts `server_` on that port.

### `void releaseResources() override`
Calls `server_->stop()`.

### `void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override`
- Syncs mode, rate, and beatDiv parameters from APVTS to `server_`.
- Detects runtime port changes and re-binds the socket server if needed.
- Ingests MIDI events from `midi`, tracking active notes, velocities, and channels in `activeNotes_`.
- Polls `getPlayHead()->getPosition()` for transport information (`timeInSeconds`, `timeInSamples`, `ppq`, `bpm`, `timeSignature`, `isPlaying`, `isRecording`, `isLooping`, `barNumber`, `beatInBar`).
- Pushes updated `PositionData` to `server_->updatePosition()`.
- Calls the protected pure virtual `processAudio(buffer)`.

### `juce::AudioProcessorEditor* createEditor() override`
Returns a newly allocated instance of `PluginEditor(*this)`.

### `bool hasEditor() const override`
Always returns `true`.

---

## Accessor Methods

```cpp
juce::AudioProcessorValueTreeState& getAPVTS();
WebSocketServer* getServer();
double getSampleRate() const;
bool isSynth() const;
int getMidiEventCount() const;
int getActiveNoteCount() const;
```

---

## Protected Virtual Methods

### `virtual void processAudio(juce::AudioBuffer<float>& buffer) = 0`
Pure virtual hook implemented by subclasses to perform audio processing or clearing.
