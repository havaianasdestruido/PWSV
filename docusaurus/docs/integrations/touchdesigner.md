---
sidebar_position: 1
title: "TouchDesigner Integration"
description: "Real-time visual generation and live stage graphics in Derivative TouchDesigner driven by PWSV telemetry."
---

# TouchDesigner Integration

Derivative TouchDesigner is widely used for live stage visuals, interactive installations, and generative projection mapping. PWSV connects directly to TouchDesigner via the **WebSocket DAT**.

---

## 1. Setting up the WebSocket DAT

1. In TouchDesigner, press `TAB` and place a **WebSocket DAT** (from the DAT palette).
2. Set the parameters:
   - **Network Address**: `127.0.0.1` (or `localhost`)
   - **Network Port**: `8080` (matching your PWSV instance)
   - **Active**: `On`
3. As soon as your DAW plays, incoming JSON messages will populate the table.

---

## 2. Parsing Messages via Python Callbacks

Attach a **DAT Execute** or write inside the `websocket1_callbacks` script:

```python
import json

def onReceiveText(dat, rowIndex, message, bytes):
    try:
        data = json.loads(message)
        
        # Route parameters to Constant CHOP or custom operator parameters
        op('constant_bpm').par.value0 = data.get('bpm', 120.0)
        op('constant_transport').par.value0 = 1 if data.get('playing') else 0
        op('constant_bar').par.value0 = data.get('bar', 1)
        op('constant_beat').par.value0 = data.get('beat', 0.0)
        
        # Trigger visual pulses on active notes
        notes = data.get('notes', [])
        for note in notes:
            # Trigger particle emitters or shaders
            pass
            
    except Exception as e:
        print("JSON Parse Error:", e)
```

---

## 3. Creating Beat-Locked Particle Triggers

- Map `data['beat']` to an LFO phase input or Ramp TOP.
- Use `data['notes']` to instantiate 3D geometry in a Geo COMP based on pitch and velocity.
