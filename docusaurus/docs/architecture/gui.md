---
sidebar_position: 4
title: "Plugin UI & APVTS Binding"
description: "User interface architecture, JUCE component layout, and APVTS parameter synchronization."
---

# Plugin UI & APVTS Binding

The graphical interface is implemented in `PluginEditor` (`PluginEditor.h` / `PluginEditor.cpp`), inheriting from `juce::AudioProcessorEditor` and implementing `juce::Timer`.

---

## Visual Design

The UI has fixed dimensions of **420 x 340 pixels** with a modern dark theme:
- Outer background: Deep navy (`#1A1A2E`)
- Inner panel: Slate blue container (`#16213E`) with rounded corners (8 px radius).
- Title: Bold 22pt centered label.

---

## Dynamic Mode Switching

The interface adapts its visible controls depending on the selected **Mode**:

| Mode | Rate Slider | Beat Division Combo Box | Units |
|---|---|---|---|
| **Hz (Mode 0)** | Visible (1.0 to 240.0) | Hidden | ` Hz` |
| **Beats (Mode 1)** | Hidden | Visible (1/1 to 3/32) | Fraction / Triplet |
| **Minutes (Mode 2)** | Visible (60.0 to 14,400.0) | Hidden | ` /min` |

The update routine toggles component visibility dynamically:
```cpp
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
```

---

## Real-Time Telemetry Timer

The editor runs a 10 Hz timer (`startTimerHz(10)`) in `timerCallback()`:
- Checks if the mode parameter was changed externally (e.g., via DAW automation) and triggers `updateSliderForMode()`.
- Updates the rate slider value if not currently being dragged by the user.
- Queries `WebSocketServer::isRunning()`, `WebSocketServer::getConnectedClientCount()`, total MIDI events, and active note count.
- Updates the status label color: **Green** when running with active client count, or **Red** if stopped.
