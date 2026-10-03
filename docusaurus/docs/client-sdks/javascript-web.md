---
sidebar_position: 2
title: "JavaScript & Web Monitor"
description: "Connecting from web applications and browsers using standard HTML5 WebSockets."
---

# JavaScript & Web Monitor

Connecting to PWSV from any web browser or Node.js environment requires zero external dependencies using the native browser `WebSocket` API.

---

## Browser Implementation (`sample/monitor.html`)

Below is the complete implementation of the web monitor provided in `sample/monitor.html`:

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Pato's WebSocket VST Monitor</title>
</head>
<body>
    <h1>Pato's WebSocket VST</h1>
    <input id="port" type="number" value="8080" min="1024" max="65535">
    <button onclick="connect()">Connect</button>
    <button onclick="disconnect()">Disconnect</button>
    <p>Status: <span id="status">Disconnected</span></p>
    <pre id="data">Waiting for data...</pre>

    <script>
        let ws = null;

        function connect() {
            const port = document.getElementById("port").value;
            if (ws) ws.close();
            document.getElementById("status").textContent = "Connecting...";
            
            ws = new WebSocket("ws://localhost:" + port);
            
            ws.onopen = function() {
                document.getElementById("status").textContent = "Connected";
            };
            
            ws.onmessage = function(e) {
                const d = JSON.parse(e.data);
                const noteNames = ["C","C#","D","D#","E","F","F#","G","G#","A","A#","B"];
                const notes = (d.notes || []).map(function(n) {
                    return noteNames[n.note % 12] + ((n.note / 12 | 0) - 1);
                }).join(" ");
                
                let out = "";
                out += "Protocol:    " + d.protocol + "\n";
                out += "Playing:     " + d.playing + "\n";
                out += "Recording:   " + d.recording + "\n";
                out += "Looping:     " + d.looping + "\n";
                out += "Time (sec):  " + d.time_sec + "\n";
                out += "Time (samp): " + d.time_samples + "\n";
                out += "PPQ:         " + d.ppq + "\n";
                out += "BPM:         " + d.bpm + "\n";
                out += "Bar:         " + d.bar + "\n";
                out += "Beat:        " + d.beat + "\n";
                out += "Time Sig:    " + d.time_sig[0] + "/" + d.time_sig[1] + "\n";
                out += "Notes:       " + (notes || "(none)");
                document.getElementById("data").textContent = out;
            };
            
            ws.onclose = function() {
                document.getElementById("status").textContent = "Disconnected";
            };
            
            ws.onerror = function() {
                document.getElementById("status").textContent = "Error";
            };
        }

        function disconnect() {
            if (ws) { ws.close(); ws = null; }
        }
    </script>
</body>
</html>
```

---

## Modern Async / React Hook Example

```typescript
import { useState, useEffect } from 'react';

export function usePWSV(port = 8080) {
  const [data, setData] = useState<any>(null);
  const [connected, setConnected] = useState(false);

  useEffect(() => {
    const ws = new WebSocket(`ws://127.0.0.1:${port}`);
    ws.onopen = () => setConnected(true);
    ws.onclose = () => setConnected(false);
    ws.onmessage = (event) => setData(JSON.parse(event.data));

    return () => ws.close();
  }, [port]);

  return { data, connected };
}
```
