import socket, time, base64, os, json, sys

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
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n"
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

def bar(beat, length=20):
    pos = int((beat % 4.0) / 4.0 * length)
    return "[" + "=" * pos + ">" + "." * (length - pos - 1) + "]"

def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
    print("Connecting to ws://localhost:%d ..." % port)
    s = connect_ws("127.0.0.1", port)
    print("Connected! Press Ctrl+C to quit.\n")

    try:
        while True:
            data = recv_frame(s)
            if data is None:
                print("Connection closed")
                break
            d = json.loads(data)

            status = ""
            if d["playing"]:
                status = "PLAYING"
            if d["recording"]:
                status = "REC"
            if not d["playing"] and not d["recording"]:
                status = "STOPPED"

            sig = "%d/%d" % (d["time_sig"][0], d["time_sig"][1])
            beat_bar = bar(d["beat"])

            note_names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
            notes = d.get("notes", [])
            note_str = ""
            if notes:
                names = []
                for n in notes:
                    note = n["note"] % 12
                    octave = (n["note"] // 12) - 1
                    names.append("%s%d" % (note_names[note], octave))
                note_str = " | Notes: " + " ".join(names)

            sys.stdout.write(
                "\r"
                " %s | %5.1fs | %6d samp | ppq %7.1f | %3.0f BPM | "
                "Bar %d Beat %.1f %s | %s %s%s   "
                % (
                    status,
                    d["time_sec"],
                    d["time_samples"],
                    d["ppq"],
                    d["bpm"],
                    d["bar"],
                    d["beat"],
                    beat_bar,
                    sig,
                    "LOOP" if d["looping"] else "",
                    note_str,
                )
            )
            sys.stdout.flush()
    except KeyboardInterrupt:
        print("\n\nDisconnected.")
    finally:
        s.close()

main()
