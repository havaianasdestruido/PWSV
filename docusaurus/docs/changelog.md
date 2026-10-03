---
sidebar_position: 11
title: "Changelog & Release Notes"
description: "Version history, protocol revisions, and roadmap."
---

# Changelog & Release Notes

All notable changes to the PWSV project are documented here.

---

## [1.1.1] - 2026-10

### Added
- Complete **Docusaurus** codebase documentation suite for `/docs`.
- Primary **Jekyll** homepage for the root website.
- Detailed C++ API documentation, Protocol v2 schema, and client SDK integration guides.
- Automated GitHub Actions deployment workflows for unified GitHub Pages hosting.

---

## [1.1.0] - 2026-09

### Added
- **Protocol v2**:
  - Structured active MIDI note broadcasting (`notes` array with pitch, velocity, and channel).
  - Explicit `"protocol": 2` header tag.
  - Transport state indicators for recording and looping.
  - Musical bar and fractional beat indices.
- **JUCE 8.0.6 Upgrade**: Migrated to JUCE 8 framework.
- **CLAP Plugin Support**: Added `clap-juce-extensions` integration for native CLAP binaries.
- **`sample/visualizer.py`**: Full 24-bit TrueColor ANSI terminal visualizer with beat grid, spectrum simulation, and timeline.
- **`sample/keys.py`**: Python MIDI note-to-key-name translator.
- **`sample/monitor_cli.py`**: Single-line terminal monitor.

### Changed
- Refactored `WebSocketServer` into a dedicated `juce::Thread` with non-blocking socket polling (`select()`).
- Upgraded installer batch script with automatic UAC administrator privilege elevation.

---

## [1.0.0] - Initial Release

- Core `PatoWebSocketEffect` and `PatoWebSocketGenerator` plugins.
- Basic RFC 6455 WebSocket streaming in Hz mode.
- Windows 7z SFX setup installer.
- HTML5 web browser monitor sample (`sample/monitor.html`).
