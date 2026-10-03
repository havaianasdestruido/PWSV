---
sidebar_position: 5
title: "GeneratorProcessor"
description: "C++ API reference for GeneratorProcessor class."
---

# `GeneratorProcessor`

Inherits from: `WebSocketProcessorBase`  
Header: `Source/GeneratorProcessor.h`  
Source: `Source/GeneratorProcessor.cpp`

`GeneratorProcessor` is the concrete implementation for the **Patos WebSocket VST Generator** virtual instrument plugin.

---

## Class Declaration

```cpp
class GeneratorProcessor : public WebSocketProcessorBase {
public:
    GeneratorProcessor() : WebSocketProcessorBase(true) {}

    const juce::String getName() const override { return "Patos WS Generator"; }
    bool acceptsMidi()  const override { return true; }
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
    void processAudio(juce::AudioBuffer<float>& buffer) override {
        buffer.clear(); // Silent generator
    }
};
```

---

## Factory Function

Defined in `Source/GeneratorProcessor.cpp`:

```cpp
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new GeneratorProcessor();
}
```
