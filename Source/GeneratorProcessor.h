#pragma once
#include "WebSocketProcessorBase.h"

class GeneratorProcessor : public WebSocketProcessorBase {
public:
    GeneratorProcessor() : WebSocketProcessorBase(true) {}

    const juce::String getName() const override { return "Patos WS Generator"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

protected:
    void processAudio(juce::AudioBuffer<float>& buffer) override {
        buffer.clear();
    }
};
