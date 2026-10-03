---
sidebar_position: 2
title: "OBS Studio Overlays"
description: "Building live stream overlays with real-time DAW transport, BPM, and piano roll widgets for OBS Studio."
---

# OBS Studio Overlays

With PWSV, live streamers and music producers can overlay real-time DAW statistics, BPM counters, and virtual piano roll animations directly inside **OBS Studio** or **Streamlabs**.

---

## 1. Creating the HTML Overlay

Create an HTML file (e.g. `overlay.html`):

```html
<!DOCTYPE html>
<html>
<head>
    <style>
        body {
            margin: 0;
            background: transparent;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            color: #fff;
        }
        .widget {
            display: inline-flex;
            align-items: center;
            gap: 12px;
            background: rgba(15, 23, 42, 0.85);
            border: 2px solid #00d2ff;
            border-radius: 12px;
            padding: 10px 20px;
            box-shadow: 0 4px 20px rgba(0, 210, 255, 0.3);
        }
        .bpm-tag {
            font-size: 24px;
            font-weight: bold;
            color: #00ffa3;
        }
        .bar-tag {
            font-size: 18px;
            color: #cbd5e1;
        }
        .pulse {
            width: 14px;
            height: 14px;
            border-radius: 50%;
            background: #ff0055;
            transition: transform 0.05s ease;
        }
        .pulse.active {
            transform: scale(1.4);
            background: #00ffa3;
        }
    </style>
</head>
<body>
    <div class="widget">
        <div id="dot" class="pulse"></div>
        <div id="bpm" class="bpm-tag">120 BPM</div>
        <div id="position" class="bar-tag">Bar 1.1</div>
    </div>

    <script>
        const ws = new WebSocket("ws://127.0.0.1:8080");
        const dot = document.getElementById("dot");
        const bpmEl = document.getElementById("bpm");
        const posEl = document.getElementById("position");

        ws.onmessage = (e) => {
            const d = JSON.parse(e.data);
            bpmEl.textContent = `${d.bpm.toFixed(0)} BPM`;
            posEl.textContent = `Bar ${d.bar}.${Math.floor(d.beat) + 1}`;
            
            if (d.playing) {
                dot.classList.toggle("active", (d.beat % 1) < 0.2);
            } else {
                dot.classList.remove("active");
            }
        };
    </script>
</body>
</html>
```

---

## 2. Adding to OBS Studio

1. In OBS, click `+` under **Sources** and select **Browser**.
2. Check **Local file** and browse to `overlay.html`.
3. Set Width to `400` and Height to `120`.
4. The overlay will render with full transparency and update automatically as you produce music in your DAW!
