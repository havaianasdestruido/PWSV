import socket, time, base64, os, json

def recv_frame(s):
    header = b""
    while len(header) < 2:
        chunk = s.recv(2 - len(header))
        if not chunk:
            return None
        header += chunk
    length = header[1] & 0x7F
    if length == 126:
        while len(header) < 4:
            chunk = s.recv(4 - len(header))
            if not chunk:
                return None
            header += chunk
        length = int.from_bytes(header[2:4], "big")
    elif length == 127:
        while len(header) < 10:
            chunk = s.recv(10 - len(header))
            if not chunk:
                return None
            header += chunk
        length = int.from_bytes(header[2:10], "big")
    payload = b""
    while len(payload) < length:
        chunk = s.recv(length - len(payload))
        if not chunk:
            return None
        payload += chunk
    return payload.decode()

s = socket.socket()
s.settimeout(5)
s.connect(('127.0.0.1', 8080))

key = base64.b64encode(os.urandom(16)).decode()
req = (
    "GET / HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Key: " + key + "\r\n"
    "Sec-WebSocket-Version: 13\r\n"
    "\r\n"
)
s.sendall(req.encode())
resp = s.recv(4096)
print("Handshake:", "OK" if b"101" in resp else "FAIL")

note_names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
for i in range(5):
    time.sleep(1)
    data = recv_frame(s)
    if data is None:
        print("Frame %d: connection closed" % i)
        break
    d = json.loads(data)
    play = "PLAYING" if d["playing"] else "stopped"
    notes = d.get("notes", [])
    note_str = ""
    if notes:
        names = ["%s%d" % (note_names[n["note"] % 12], n["note"] // 12 - 1) for n in notes]
        note_str = " notes=" + ",".join(names)
    print("Frame %d: proto=%d t=%.1fs ppq=%.1f bpm=%.0f bar=%d beat=%.1f [%s]%s" % (
        i, d["protocol"], d["time_sec"], d["ppq"], d["bpm"], d["bar"], d["beat"], play, note_str))
s.close()
