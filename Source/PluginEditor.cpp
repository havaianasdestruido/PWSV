#include "PluginEditor.h"

PluginEditor::PluginEditor(WebSocketProcessorBase& p)
    : AudioProcessorEditor(p), proc_(p)
{
    setSize(420, 340);

    auto& apvts = proc_.getAPVTS();

    // ── title ──────────────────────────────────────────────
    titleLabel_.setText("Pato's WebSocket VST", juce::dontSendNotification);
    titleLabel_.setFont(juce::Font(22.0f, juce::Font::bold));
    titleLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel_);

    // ── mode ──────────────────────────────────────────────
    modeLabel_.setText("Mode:", juce::dontSendNotification);
    modeLabel_.setFont(juce::Font(14.0f, juce::Font::plain));
    addAndMakeVisible(modeLabel_);

    modeBox_.addItemList({ "Hz", "Beats", "Minutes" }, 1);
    modeBox_.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(modeBox_);
    modeAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "mode", modeBox_);

    modeBox_.onChange = [this] { updateSliderForMode(modeBox_.getSelectedId() - 1); };

    // ── rate slider ────────────────────────────────────────
    rateLabel_.setText("Rate:", juce::dontSendNotification);
    rateLabel_.setFont(juce::Font(14.0f, juce::Font::plain));
    addAndMakeVisible(rateLabel_);

    rateSlider_.setRange(1.0, 240.0, 0.1);
    rateSlider_.setValue(10.0, juce::dontSendNotification);
    rateSlider_.setTextValueSuffix(" Hz");
    rateSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(rateSlider_);

    rateSlider_.onValueChange = [this] {
        int mode = modeBox_.getSelectedId() - 1;
        double val = rateSlider_.getValue();
        float hz = (mode == 2) ? static_cast<float>(val / 60.0) : static_cast<float>(val);
        auto* p = proc_.getAPVTS().getParameter("rate");
        p->setValueNotifyingHost(p->convertTo0to1(hz));
    };

    // ── beat division ─────────────────────────────────────
    beatDivLabel_.setText("Division:", juce::dontSendNotification);
    beatDivLabel_.setFont(juce::Font(14.0f, juce::Font::plain));
    addAndMakeVisible(beatDivLabel_);

    for (int i = 0; i < WebSocketServer::NUM_BEAT_DIVISIONS; ++i)
        beatDivBox_.addItem(WebSocketServer::beatDivisionLabels[i], i + 1);
    beatDivBox_.setSelectedId(3, juce::dontSendNotification);
    addAndMakeVisible(beatDivBox_);
    beatDivAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "beatDiv", beatDivBox_);

    // ── port ──────────────────────────────────────────────
    portLabel_.setText("Port:", juce::dontSendNotification);
    portLabel_.setFont(juce::Font(14.0f, juce::Font::plain));
    addAndMakeVisible(portLabel_);

    portSlider_.setRange(1024.0, 65535.0, 1.0);
    portSlider_.setValue(8080.0, juce::dontSendNotification);
    portSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(portSlider_);
    portAttach_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "port", portSlider_);

    // ── status ─────────────────────────────────────────────
    statusLabel_.setText("Starting server...", juce::dontSendNotification);
    statusLabel_.setFont(juce::Font(12.0f, juce::Font::plain));
    statusLabel_.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(statusLabel_);

    // ── version ───────────────────────────────────────────
    versionLabel_.setText("v1.1.1", juce::dontSendNotification);
    versionLabel_.setFont(juce::Font(10.0f, juce::Font::plain));
    versionLabel_.setJustificationType(juce::Justification::centredRight);
    versionLabel_.setColour(juce::Label::textColourId, juce::Colour(0xFF555555));
    addAndMakeVisible(versionLabel_);

    updateSliderForMode(0);
    startTimerHz(10);
}

PluginEditor::~PluginEditor() {
    stopTimer();
}

void PluginEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF1A1A2E));
    g.setColour(juce::Colour(0xFF16213E));
    g.fillRoundedRectangle(10, 10, getWidth() - 20, getHeight() - 20, 8);
}

void PluginEditor::resized() {
    auto area = getLocalBounds().reduced(16);
    int rowH  = 30;
    int labelW = 70;

    titleLabel_.setBounds(area.removeFromTop(40));
    area.removeFromTop(8);

    {
        auto row = area.removeFromTop(rowH);
        modeLabel_.setBounds(row.removeFromLeft(labelW));
        modeBox_.setBounds(row);
    }
    area.removeFromTop(6);

    {
        auto row = area.removeFromTop(rowH);
        rateLabel_.setBounds(row.removeFromLeft(labelW));
        rateSlider_.setBounds(row);
    }
    area.removeFromTop(6);

    {
        auto row = area.removeFromTop(rowH);
        beatDivLabel_.setBounds(row.removeFromLeft(labelW));
        beatDivBox_.setBounds(row);
    }
    area.removeFromTop(6);

    {
        auto row = area.removeFromTop(rowH);
        portLabel_.setBounds(row.removeFromLeft(labelW));
        portSlider_.setBounds(row);
    }
    area.removeFromTop(10);

    statusLabel_.setBounds(area.removeFromTop(20));
    area.removeFromTop(4);
    versionLabel_.setBounds(area.removeFromBottom(16));
}

void PluginEditor::timerCallback() {
    int mode = static_cast<int>(proc_.getAPVTS().getRawParameterValue("mode")->load());

    if (mode != lastMode_) {
        lastMode_ = mode;
        updateSliderForMode(mode);
    }

    if (!rateSlider_.isMouseButtonDown() && mode != 1) {
        float hz = proc_.getAPVTS().getRawParameterValue("rate")->load();
        double disp = (mode == 2) ? static_cast<double>(hz) * 60.0 : static_cast<double>(hz);
        if (std::abs(rateSlider_.getValue() - disp) > 0.05)
            rateSlider_.setValue(disp, juce::dontSendNotification);
    }

    auto* server = proc_.getServer();
    if (server && server->isRunning()) {
        int n = server->getConnectedClientCount();
        statusLabel_.setText("Server running on port " +
            juce::String(static_cast<int>(portSlider_.getValue())) +
            " - " + juce::String(n) + " client" + (n == 1 ? "" : "s") +
            " | MIDI: " + juce::String(proc_.getMidiEventCount()) +
            " | Active: " + juce::String(proc_.getActiveNoteCount()),
            juce::dontSendNotification);
        statusLabel_.setColour(juce::Label::textColourId, juce::Colours::green);
    } else {
        statusLabel_.setText("Server stopped", juce::dontSendNotification);
        statusLabel_.setColour(juce::Label::textColourId, juce::Colours::red);
    }
}

void PluginEditor::updateSliderForMode(int mode) {
    switch (mode) {
        case 0: // Hz
            rateSlider_.setVisible(true);
            rateLabel_.setVisible(true);
            beatDivBox_.setVisible(false);
            beatDivLabel_.setVisible(false);
            rateSlider_.setRange(1.0, 240.0, 0.1);
            rateSlider_.setTextValueSuffix(" Hz");
            break;
        case 1: // Beats
            rateSlider_.setVisible(false);
            rateLabel_.setVisible(false);
            beatDivBox_.setVisible(true);
            beatDivLabel_.setVisible(true);
            break;
        case 2: // Minutes
            rateSlider_.setVisible(true);
            rateLabel_.setVisible(true);
            beatDivBox_.setVisible(false);
            beatDivLabel_.setVisible(false);
            rateSlider_.setRange(60.0, 14400.0, 1.0);
            rateSlider_.setTextValueSuffix(" /min");
            break;
    }
    resized();
}
