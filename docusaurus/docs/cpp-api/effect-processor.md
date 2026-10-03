---
sidebar_position: 4
title: "EffectProcessor"
description: "C++ API reference for EffectProcessor class."
---

# `EffectProcessor`

Inherits from: `WebSocketProcessorBase`  
Header: `Source/EffectProcessor.h`  
Source: `Source/EffectProcessor.cpp`

`EffectProcessor` is the concrete implementation for the **Patos WebSocket VST Effect** audio plugin.

---

## Class Declaration

```cpp
class EffectProcessor : public WebSocketProcessorBase {
public:
    EffectProcessor() : WebSocketProcessorBase(false) {}

    const juce::String getName() const override { return "Patos WS Effect"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

protected:
    void processAudio(juce::AudioBuffer<float>&) override {
        // Audio pass-through (no processing)
    }
};
```

---

## Factory Function

Defined in `Source/EffectProcessor.cpp` for JUCE plugin entry:

```cpp
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new EffectProcessor();
}
```
