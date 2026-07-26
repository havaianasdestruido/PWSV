#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "WebSocketProcessorBase.h"

class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer {
public:
    explicit PluginEditor(WebSocketProcessorBase&);
    ~PluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateSliderForMode(int mode);

    WebSocketProcessorBase& proc_;

    juce::Label   titleLabel_;
    juce::Label   modeLabel_;
    juce::ComboBox modeBox_;
    juce::Label   rateLabel_;
    juce::Slider  rateSlider_;
    juce::Label   beatDivLabel_;
    juce::ComboBox beatDivBox_;
    juce::Label   portLabel_;
    juce::Slider  portSlider_;
    juce::Label   statusLabel_;
    juce::Label   versionLabel_;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> beatDivAttach_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   portAttach_;

    int lastMode_ = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
