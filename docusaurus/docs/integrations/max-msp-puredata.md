---
sidebar_position: 4
title: "Max/MSP & Pure Data"
description: "Interfacing PWSV with cycling '74 Max/MSP and Miller Puckette's Pure Data."
---

# Max/MSP & Pure Data Integration

For modular sound designers and live coders, PWSV can stream telemetry directly into Cycling '74 **Max/MSP** and **Pure Data (Pd)**.

---

## 1. Max/MSP (Node for Max)

Using `node.script` inside Max 8 or Max 9:

### Installation
In the directory containing your Max patch, install the WebSocket package:
```bash
npm install ws
```

### `pwsv_max.js`
```javascript
const maxApi = require('max-api');
const WebSocket = require('ws');

const ws = new WebSocket('ws://127.0.0.1:8080');

ws.on('open', () => {
    maxApi.outlet('status', 'connected');
});

ws.on('message', (data) => {
    try {
        const json = JSON.parse(data);
        maxApi.outlet('bpm', json.bpm);
        maxApi.outlet('bar', json.bar);
        maxApi.outlet('beat', json.beat);
        maxApi.outlet('playing', json.playing ? 1 : 0);
        
        if (json.notes && json.notes.length > 0) {
            json.notes.forEach(n => {
                maxApi.outlet('note', n.note, n.vel, n.ch);
            });
        }
    } catch (e) {
        maxApi.post('Error parsing JSON: ' + e);
    }
});
```

### Max Patch Setup
1. Create a `[node.script pwsv_max.js]` object.
2. Connect a `[route bpm bar beat playing note]` to parse out values to your synth or Jitter matrix patches.

---

## 2. Pure Data (Pd)

In Pure Data, use the `[iemnet/tcpclient]` external or a lightweight Python bridge forwarding to `[netreceive]` via OSC/UDP.
