---
sidebar_position: 4
title: "Terminal ANSI Visualizer"
description: "Full interactive Python ANSI color visualizer featuring beat grids, spectrum animations, and timeline history."
---

# Terminal ANSI Visualizer

`sample/visualizer.py` is a rich, cross-platform terminal visualizer built purely in Python standard library using ANSI 24-bit TrueColor escape codes.

---

## Visualizer Interface

```text
  ╔════════════════════════════════════════════════════════════════════════════╗
  ║ PATO'S WEBSOCKET VST - LIVE VISUALIZER                                     ║
  ╠════════════════════════════════════════════════════════════════════════════╣

    PLAYING   BPM 128     TIME 0:42.50   BAR 14   BEAT 2.0   4/4 

    BEAT GRID
    ██ ██ •  •  ██ ██ •  •  

    SPECTRUM
    ██    ██    ██    ██
    ██ ██ ██ ██ ██ ██ ██ ██
    ██ ██ ██ ██ ██ ██ ██ ██
    ██ ██ ██ ██ ██ ██ ██ ██
    ██ ██ ██ ██ ██ ██ ██ ██

    TIMELINE
    ████████████░░░░░░░░████████████

    ▶ PPQ: 54.0  Samples: 2381400
  ╚════════════════════════════════════════════════════════════════════════════╝
```

---

## Running the Visualizer

Open your terminal and run:

```bash
# Default connects to port 8080
python sample/visualizer.py

# Or specify a custom port
python sample/visualizer.py 8081
```

---

## Technical Components of the Visualizer

1. **Header Panel**: Displays transport status (PLAYING in Green, RECORD in Red, STOPPED in Dim Grey), BPM, elapsed minutes:seconds, musical bar, current beat, and time signature.
2. **Dynamic Beat Grid**: Subdivides the measure by time signature numerator and illuminates current beat with an animated pulse decay.
3. **Simulated Spectrum Analyzer**: Multi-band EQ visualization that modulates with transport phase, BPM tempo harmonics, and quarter-note beat transients.
4. **Scrolling Bar Timeline**: Renders beat intensity history for the current musical bar, clearing automatically on bar transitions.
5. **Footer Transport Bar**: Indicates play/pause status, loop region status, total PPQ, and exact sample frame counter.
