#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "WebSocketServer.h"
#include <set>

class WebSocketProcessorBase : public juce::AudioProcessor {
public:
    explicit WebSocketProcessorBase(bool isSynth);
    ~WebSocketProcessorBase() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    juce::AudioProcessorValueTreeState& getAPVTS() { return *apvts_; }
    WebSocketServer* getServer() { return server_.get(); }

    double getSampleRate() const { return sampleRate_; }
    bool isSynth() const { return isSynth_; }

protected:
    virtual void processAudio(juce::AudioBuffer<float>& buffer) = 0;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts_;
    std::unique_ptr<WebSocketServer> server_;
    double sampleRate_ = 44100.0;
    int currentPort_   = 8080;
    bool isSynth_;

    struct ActiveNote { int note; int velocity; int channel; };
    std::vector<ActiveNote> activeNotes_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebSocketProcessorBase)
};
