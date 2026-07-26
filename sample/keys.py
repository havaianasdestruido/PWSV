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
req = ("GET / HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Version: 13\r\n\r\n" % key)
s.sendall(req.encode())
s.recv(4096)

names = ["C","C#","D","D#","E","F","F#","G","G#","A","A#","B"]
try:
    while True:
        data = recv_frame(s)
        if not data:
            break
        notes = json.loads(data).get("notes", [])
        keys = ["%s%d" % (names[n["note"] % 12], n["note"] // 12 - 1) for n in notes]
        print(keys)
except KeyboardInterrupt:
    pass
s.close()
