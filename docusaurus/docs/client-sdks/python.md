---
sidebar_position: 3
title: "Python Raw Socket Clients"
description: "Connecting from Python using built-in standard library sockets (no pip dependencies required)."
---

# Python Raw Socket Clients

PWSV provides sample Python scripts that use Python's built-in `socket` library to execute the RFC 6455 opening handshake and receive data frames without requiring any `pip` packages (such as `websockets`).

---

## 1. Minimal Frame Receiver (`sample/test_ws.py`)

```python
import socket, base64, os, json, sys

def connect_ws(host, port):
    s = socket.socket()
    s.settimeout(5)
    s.connect((host, port))
    key = base64.b64encode(os.urandom(16)).decode()
    req = (
        "GET / HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %s\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n"
    ) % (host, port, key)
    s.sendall(req.encode())
    resp = s.recv(4096)
    if b"101" not in resp:
        print("Handshake failed")
        sys.exit(1)
    return s

def recv_frame(s):
    header = b""
    while len(header) < 2:
        chunk = s.recv(2 - len(header))
        if not chunk: return None
        header += chunk
    length = header[1] & 0x7F
    if length == 126:
        while len(header) < 4:
            chunk = s.recv(4 - len(header))
            if not chunk: return None
            header += chunk
        length = int.from_bytes(header[2:4], "big")
    elif length == 127:
        while len(header) < 10:
            chunk = s.recv(10 - len(header))
            if not chunk: return None
            header += chunk
        length = int.from_bytes(header[2:10], "big")
    
    payload = b""
    while len(payload) < length:
        chunk = s.recv(length - len(payload))
        if not chunk: return None
        payload += chunk
    return payload.decode()

s = connect_ws('127.0.0.1', 8080)
try:
    while True:
        data = recv_frame(s)
        if not data: break
        d = json.loads(data)
        print(f"BPM: {d['bpm']} | Bar: {d['bar']} | Beat: {d['beat']} | Playing: {d['playing']}")
except KeyboardInterrupt:
    s.close()
```

---

## 2. MIDI Note Name Translator (`sample/keys.py`)

Translates raw MIDI note numbers (0–127) into musical octave notes (e.g. `C4`, `F#3`).

*(This script relies on `connect_ws` and `recv_frame` defined above in `sample/test_ws.py`)*:

```python
import socket, json
from test_ws import connect_ws, recv_frame

s = connect_ws('127.0.0.1', 8080)
names = ["C","C#","D","D#","E","F","F#","G","G#","A","A#","B"]

try:
    while True:
        data = recv_frame(s)
        if not data: break
        notes = json.loads(data).get("notes", [])
        keys = ["%s%d" % (names[n["note"] % 12], n["note"] // 12 - 1) for n in notes]
        print(keys)
except KeyboardInterrupt:
    s.close()
```

---

## 3. Real-Time Monitor CLI (`sample/monitor_cli.py`)

Provides a single-line terminal transport monitor that overwrites the current line in real time:

```bash
python sample/monitor_cli.py 8080
```
Output:
```text
BPM: 128.0 | Bar: 14 | Beat: 3.25 | PLAYING | Notes: C4 E4 G4
```
