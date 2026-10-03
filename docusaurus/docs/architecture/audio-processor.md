---
sidebar_position: 2
title: "Audio Processor & Threading"
description: "Deep dive into the audio processing lifecycle, APVTS parameters, and real-time safe state management."
---

# Audio Processor & Threading

The audio processing core of PWSV is implemented in `WebSocketProcessorBase`, which inherits from `juce::AudioProcessor`.

---

## Class Hierarchy

```text
       juce::AudioProcessor
                 │
                 ▼
      WebSocketProcessorBase  ◄── owns ──► AudioProcessorValueTreeState (APVTS)
        │                 │   ◄── owns ──► WebSocketServer
        ▼                 ▼
 EffectProcessor   GeneratorProcessor
```

---

## Processor Lifecycle

### 1. Construction
When the host instantiates the plugin:
```cpp
WebSocketProcessorBase::WebSocketProcessorBase(bool isSynth)
    : AudioProcessor(isSynth
          ? BusesProperties()
                .withOutput("Output", juce::AudioChannelSet::stereo(), true)
          : BusesProperties()
                .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
                .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(std::make_unique<juce::AudioProcessorValueTreeState>(
          *this, nullptr, "Parameters", createParameterLayout())),
      server_(std::make_unique<WebSocketServer>()),
      isSynth_(isSynth)
{
}
```
- Bus layouts are assigned depending on whether the plugin is configured as a Synth (`Generator`) or Effect.
- The `AudioProcessorValueTreeState` (APVTS) parameter layout is constructed.
- An instance of `WebSocketServer` is allocated.

### 2. `prepareToPlay`
Called by the DAW before playback starts or when sample rate changes:
```cpp
void WebSocketProcessorBase::prepareToPlay(double sr, int) {
    sampleRate_ = sr;
    int port = static_cast<int>(apvts_->getRawParameterValue("port")->load());
    currentPort_ = port;
    server_->start(port);
}
```
- Initializes the sample rate.
- Reads the active port parameter and starts the `WebSocketServer` background thread.

### 3. `processBlock`
Called continuously for every audio buffer block:
```cpp
void WebSocketProcessorBase::processBlock(
    juce::AudioBuffer<float>& buffer, 
    juce::MidiBuffer& midi) 
{
    juce::ScopedNoDenormals noDenormals;

    // 1. Sync parameters to server atomically
    int mode    = static_cast<int>(apvts_->getRawParameterValue("mode")->load());
    float rate  = apvts_->getRawParameterValue("rate")->load();
    int beatDiv = static_cast<int>(apvts_->getRawParameterValue("beatDiv")->load());

    server_->setMode(mode);
    server_->setRate(rate);
    server_->setBeatDivision(beatDiv);

    // 2. Handle dynamic port changes
    int port = static_cast<int>(apvts_->getRawParameterValue("port")->load());
    if (port != currentPort_) {
        currentPort_ = port;
        server_->stop();
        server_->start(port);
    }

    // 3. Process MIDI notes
    for (const auto metadata : midi) {
        auto msg = metadata.getMessage();
        midiEventCount_.fetch_add(1);
        int note = msg.getNoteNumber();
        int ch   = msg.getChannel();
        if (msg.isNoteOn()) {
            activeNotes_.push_back({ note, msg.getVelocity(), ch });
        } else if (msg.isNoteOff()) {
            for (auto it = activeNotes_.begin(); it != activeNotes_.end(); ) {
                if (it->note == note && it->channel == ch)
                    it = activeNotes_.erase(it);
                else
                    ++it;
            }
        }
    }
    activeNoteCount_.store(static_cast<int>(activeNotes_.size()));

    // 4. Query DAW playhead position
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
                double barLengthPpq = (pd.timeSigDen > 0)
                    ? (static_cast<double>(pd.timeSigNum) * 4.0 / pd.timeSigDen)
                    : 4.0;
                pd.barNumber = static_cast<int>(*barStart / barLengthPpq) + 1;
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

    // 5. Invoke virtual audio processor
    processAudio(buffer);
}
```

### 4. `releaseResources`
Called when playback stops or the plugin is deactivated:
```cpp
void WebSocketProcessorBase::releaseResources() {
    server_->stop();
}
```
Signals the background server thread to stop and cleanly shuts down all active client sockets.
