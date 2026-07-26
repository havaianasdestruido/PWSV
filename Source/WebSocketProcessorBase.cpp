#include "WebSocketProcessorBase.h"
#include "PluginEditor.h"

WebSocketProcessorBase::WebSocketProcessorBase(bool isSynth)
    : AudioProcessor(isSynth
          ? BusesProperties()
                .withOutput("Output", juce::AudioChannelSet::stereo(), true)
          : BusesProperties()
                .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
                .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(std::make_unique<juce::AudioProcessorValueTreeState>(*this, nullptr, "Parameters", createParameterLayout())),
      server_(std::make_unique<WebSocketServer>()),
      isSynth_(isSynth)
{
}

WebSocketProcessorBase::~WebSocketProcessorBase() {
    server_->stop();
}

void WebSocketProcessorBase::prepareToPlay(double sr, int) {
    sampleRate_ = sr;
    int port = static_cast<int>(apvts_->getRawParameterValue("port")->load());
    currentPort_ = port;
    server_->start(port);
}

void WebSocketProcessorBase::releaseResources() {
    server_->stop();
}

void WebSocketProcessorBase::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;

    int mode    = static_cast<int>(apvts_->getRawParameterValue("mode")->load());
    float rate  = apvts_->getRawParameterValue("rate")->load();
    int beatDiv = static_cast<int>(apvts_->getRawParameterValue("beatDiv")->load());

    server_->setMode(mode);
    server_->setRate(rate);
    server_->setBeatDivision(beatDiv);

    int port = static_cast<int>(apvts_->getRawParameterValue("port")->load());
    if (port != currentPort_) {
        currentPort_ = port;
        server_->stop();
        server_->start(port);
    }

    for (const auto metadata : midi) {
        auto msg = metadata.getMessage();
        int note = msg.getNoteNumber();
        if (msg.isNoteOn()) {
            activeNotes_.push_back({ note, msg.getVelocity(), static_cast<int>(msg.getChannel()) });
        } else if (msg.isNoteOff()) {
            for (auto it = activeNotes_.begin(); it != activeNotes_.end(); ) {
                if (it->note == note)
                    it = activeNotes_.erase(it);
                else
                    ++it;
            }
        }
    }

    auto playHead = getPlayHead();
    if (playHead) {
        auto info = playHead->getPosition();
        if (info) {
            PositionData pd;
            pd.timeInSeconds = info->getTimeInSeconds().orFallback(0.0);
            pd.timeInSamples = info->getTimeInSamples().orFallback(0);
            pd.ppq           = info->getPpqPosition().orFallback(0.0);
            pd.bpm           = info->getBpm().orFallback(120.0);
            pd.isPlaying     = info->getIsPlaying();
            pd.isRecording   = info->getIsRecording();
            pd.isLooping     = info->getIsLooping();

            auto ts = info->getTimeSignature();
            if (ts) { pd.timeSigNum = ts->numerator; pd.timeSigDen = ts->denominator; }

            auto barStart = info->getPpqPositionOfLastBarStart();
            if (barStart && pd.timeSigNum > 0 && pd.ppq >= *barStart) {
                pd.barNumber = static_cast<int>(*barStart / pd.timeSigNum) + 1;
                pd.beatInBar = pd.ppq - *barStart;
            }

            pd.activeNoteCount = static_cast<int>(activeNotes_.size());
            for (int i = 0; i < pd.activeNoteCount && i < PositionData::MAX_NOTES; ++i) {
                pd.activeNoteNumbers[i]  = activeNotes_[i].note;
                pd.activeNoteVelocities[i] = activeNotes_[i].velocity;
                pd.activeNoteChannels[i] = activeNotes_[i].channel;
            }

            server_->updatePosition(pd);
            server_->setBpm(pd.bpm);
        }
    }

    processAudio(buffer);
}

juce::AudioProcessorEditor* WebSocketProcessorBase::createEditor() {
    return new PluginEditor(*this);
}

juce::AudioProcessorValueTreeState::ParameterLayout
WebSocketProcessorBase::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "mode", 1 }, "Mode",
        juce::StringArray{ "Hz", "Beats", "Minutes" }, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "rate", 1 }, "Rate",
        juce::NormalisableRange<float>(1.0f, 240.0f, 0.1f), 10.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "beatDiv", 1 }, "Beat Division",
        juce::StringArray{ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32",
                           "3/4", "3/8", "3/16", "3/32" }, 2));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "port", 1 }, "Port",
        juce::NormalisableRange<float>(1024.0f, 65535.0f, 1.0f), 8080.0f));

    return layout;
}
