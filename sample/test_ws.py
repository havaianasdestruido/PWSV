import socket, time, base64, os, json

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

for i in range(5):
    time.sleep(1)
    try:
        data = s.recv(65536)
        if not data:
            print("Frame %d: connection closed" % i)
            break
        length = data[1] & 0x7F
        offset = 2
        if length == 126:
            length = int.from_bytes(data[2:4], "big")
            offset = 4
        elif length == 127:
            length = int.from_bytes(data[2:10], "big")
            offset = 10
        payload = data[offset:offset+length].decode()
        d = json.loads(payload)
        play = "PLAYING" if d["playing"] else "stopped"
        print("Frame %d: t=%.1fs ppq=%.1f bpm=%.0f bar=%d beat=%.1f [%s]" % (i, d["time_sec"], d["ppq"], d["bpm"], d["bar"], d["beat"], play))
    except Exception as e:
        print("Frame %d: error %s" % (i, e))
        break
s.close()
