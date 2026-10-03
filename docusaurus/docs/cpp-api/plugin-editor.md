---
sidebar_position: 6
title: "PluginEditor"
description: "C++ API reference for PluginEditor class."
---

# `PluginEditor`

Inherits from: `juce::AudioProcessorEditor`, `private juce::Timer`  
Header: `Source/PluginEditor.h`  
Source: `Source/PluginEditor.cpp`

`PluginEditor` provides the graphical interface for PWSV plugins, featuring controls for mode selection, broadcast rate, musical beat division, server port, and live connection telemetry.

---

## Public Methods

### `explicit PluginEditor(WebSocketProcessorBase&)`
Constructs the UI editor. Configures the default size ($420 \times 340$ px), attaches UI widgets to APVTS parameters via `ComboBoxAttachment` and `SliderAttachment`, sets up callbacks, and calls `startTimerHz(10)`.

### `~PluginEditor() override`
Destructor. Invokes `stopTimer()` to ensure the timer stops before UI components are destroyed.

### `void paint(juce::Graphics&) override`
Renders the dark slate background (`#1A1A2E`) and rounded central card (`#16213E`).

### `void resized() override`
Calculates component layout bounds in vertical strips for all labels, dropdowns, and sliders.

---

## Private Methods & Members

### `void timerCallback() override`
Executes 10 times per second to update GUI state:
- Synchronizes dynamic mode-specific slider ranges and visibility.
- Updates the status label showing server port, connected client count, total MIDI event count, and active note count.

### `void updateSliderForMode(int mode)`
Switches the visibility and range of UI controls based on active mode (`0`: Hz, `1`: Beats, `2`: Minutes).
