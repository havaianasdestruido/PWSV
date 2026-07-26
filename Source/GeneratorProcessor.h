#pragma once
#include "WebSocketProcessorBase.h"

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
        buffer.clear();
    }
};
