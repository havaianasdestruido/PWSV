import socket, time, base64, os, json, sys, math, random, io

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")
sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding="utf-8", errors="replace")

RESET  = "\033[0m"
BOLD   = "\033[1m"
DIM    = "\033[2m"
BG     = "\033[48;2;"
FG     = "\033[38;2;"
HIDE   = "\033[?25l"
SHOW   = "\033[?25h"
CLS    = "\033[2J\033[H"

def rgb(r, g, b):
    return "%d;%d;%dm" % (r, g, b)

def fg(r, g, b):
    return FG + rgb(r, g, b)

def bg(r, g, b):
    return BG + rgb(r, g, b)

def hsl_to_rgb(h, s, l):
    h = h % 360.0
    c = (1.0 - abs(2.0 * l - 1.0)) * s
    x = c * (1.0 - abs((h / 60.0) % 2.0 - 1.0))
    m = l - c / 2.0
    if h < 60:   r, g, b = c, x, 0
    elif h < 120: r, g, b = x, c, 0
    elif h < 180: r, g, b = 0, c, x
    elif h < 240: r, g, b = 0, x, c
    elif h < 300: r, g, b = x, 0, c
    else:         r, g, b = c, 0, x
    return int((r + m) * 255), int((g + m) * 255), int((b + m) * 255)

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

BAR_CHARS = " \u2581\u2582\u2583\u2584\u2585\u2586\u2587\u2588"
BLOCK = "\u2588"

def draw_header(d, width):
    status = "PLAYING " if d["playing"] else "STOPPED "
    if d["recording"]:
        status = fg(255, 60, 60) + BOLD + " RECORD  " + RESET

    t = d["time_sec"]
    m = int(t // 60)
    s = t % 60
    time_str = "%d:%05.2f" % (m, s)

    lines = []
    lines.append("")
    lines.append(fg(0, 180, 255) + BOLD + "  \u2554" + "\u2550" * (width - 4) + "\u2557" + RESET)
    lines.append(fg(0, 180, 255) + "  \u2551 " + RESET + BOLD + fg(255, 255, 255) + "PATO'S WEBSOCKET VST - LIVE VISUALIZER" + RESET + " " * (width - 42) + fg(0, 180, 255) + "\u2551" + RESET)
    lines.append(fg(0, 180, 255) + "  \u2560" + "\u2550" * (width - 4) + "\u2563" + RESET)
    lines.append("")

    info = "  %s  %s  %s  %s  %s  %s " % (
        fg(255, 255, 255) + status + RESET,
        fg(0, 220, 120) + "BPM " + BOLD + "%-5.0f" % d["bpm"] + RESET,
        fg(255, 200, 50) + "TIME " + BOLD + time_str + RESET,
        fg(180, 120, 255) + "BAR " + BOLD + "%d" % d["bar"] + RESET,
        fg(255, 100, 200) + "BEAT " + BOLD + "%.1f" % d["beat"] + RESET,
        fg(150, 150, 150) + "%d/%d" % (d["time_sig"][0], d["time_sig"][1]) + RESET,
    )
    lines.append(info)
    lines.append("")
    return lines

def draw_beat_grid(d, width):
    lines = []
    beat = d["beat"]
    sig = d["time_sig"][0]
    playing = d["playing"]

    lines.append(fg(80, 80, 80) + "  BEAT GRID" + RESET)
    lines.append("")

    grid_w = width - 4
    num_beats = sig
    beats_displayed = max(num_beats, 8)

    row = "  "
    for i in range(beats_displayed):
        beat_pos = (i % num_beats)
        is_downbeat = (beat_pos == 0)
        current = int(beat) % num_beats
        is_current = (i % num_beats == current) and playing

        frac = beat - int(beat)
        intensity = max(0, 1.0 - frac * 2) if is_current else 0

        if is_current:
            r, g, b = hsl_to_rgb(180, 1.0, 0.5 + intensity * 0.3)
            row += fg(r, g, b) + BOLD + "\u2588\u2588" + RESET
        elif is_downbeat:
            row += fg(80, 80, 80) + "\u2500\u2500" + RESET
        else:
            row += fg(40, 40, 40) + "\u2022 " + RESET
        row += " "
    lines.append(row)
    lines.append("")

    return lines

def draw_spectrum(d, width, frame_num):
    lines = []
    lines.append(fg(80, 80, 80) + "  SPECTRUM" + RESET)
    lines.append("")

    bpm = d["bpm"]
    beat = d["beat"]
    bar = d["bar"]
    playing = d["playing"]
    frac = beat - int(beat)

    num_bars = (width - 4) // 2
    heights = []
    for i in range(num_bars):
        if not playing:
            h = 1
        else:
            phase1 = math.sin((beat + i * 0.5) * math.pi * 2.0 / 4.0 + frame_num * 0.1) * 0.3
            phase2 = math.sin((beat + i * 0.3) * math.pi * 2.0 / 2.0 - frame_num * 0.15) * 0.2
            phase3 = math.sin((beat * 0.5 + i * 0.8) * math.pi * 2.0 + frame_num * 0.05) * 0.15
            beat_pulse = max(0, 1.0 - frac * 3.0) * 0.25
            base = 0.15 + phase1 + phase2 + phase3 + beat_pulse
            h = max(0, min(8, int(base * 8)))
        heights.append(h)

    for row in range(7, -1, -1):
        line = "  "
        for i, h in enumerate(heights):
            if h > row:
                hue = (i * 12 + frame_num * 3 + row * 15) % 360
                r, g, b = hsl_to_rgb(hue, 0.8, 0.5)
                line += fg(r, g, b) + BLOCK + BLOCK + RESET
            elif h == row:
                r, g, b = hsl_to_rgb((i * 12 + frame_num * 3) % 360, 0.6, 0.3)
                line += fg(r, g, b) + "\u2591\u2591" + RESET
            else:
                line += fg(20, 20, 30) + "\u2022 " + RESET
        lines.append(line)
    lines.append("")

    return lines

def draw_timeline(d, width, history):
    lines = []
    lines.append(fg(80, 80, 80) + "  TIMELINE" + RESET)
    lines.append("")

    tl_width = width - 4
    tl = "  "

    history_len = len(history)
    visible = min(tl_width, max(history_len, 1))
    start = max(0, history_len - visible)

    for i in range(visible):
        idx = start + i
        if idx < history_len:
            h = history[idx]
            if h > 0:
                r, g, b = hsl_to_rgb(h * 30, 0.9, 0.5)
                tl += fg(r, g, b) + BLOCK + RESET
            else:
                tl += fg(30, 30, 40) + "\u2502" + RESET
        else:
            tl += fg(30, 30, 40) + "\u2502" + RESET
    lines.append(tl)
    lines.append("")

    return lines

def draw_footer(d, width):
    sig = d["time_sig"]
    num = sig[0]
    den = sig[1]

    notes = ["\u2669", "\u266A", "\u266B", "\u266C"]

    playing = d["playing"]
    looping = d["looping"]

    line1 = "  "
    if playing:
        line1 += fg(0, 200, 0) + BOLD + "\u25B6 " + RESET
    else:
        line1 += fg(100, 100, 100) + "\u25A0 " + RESET

    if looping:
        line1 += fg(255, 200, 0) + "\u21BB LOOP " + RESET

    ppq = d["ppq"]
    line1 += fg(60, 60, 80) + "PPQ: %.1f" % ppq + RESET
    line1 += fg(60, 60, 80) + "  Samples: %d" % d["time_samples"] + RESET

    pad = width - len(line1.replace("\033", "").replace("[", "").replace("m", "").replace("0", "")) - 4
    if pad > 0:
        line1 += " " * pad

    lines = []
    lines.append(fg(0, 180, 255) + "  \u255A" + "\u2550" * (width - 4) + "\u255D" + RESET)

    return lines

def render(d, width, frame_num, history):
    lines = []
    lines.extend(draw_header(d, width))
    lines.extend(draw_beat_grid(d, width))
    lines.extend(draw_spectrum(d, width, frame_num))
    lines.extend(draw_timeline(d, width, history))
    lines.extend(draw_footer(d, width))

    sys.stdout.write(CLS + HIDE)
    sys.stdout.write("\n".join(lines))
    sys.stdout.write("\n" + RESET + SHOW)
    sys.stdout.flush()

def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
    try:
        import shutil
        width = shutil.get_terminal_size((80, 24)).columns
    except:
        width = 80
    width = max(60, min(width, 120))

    print(CLS + fg(0, 180, 255) + BOLD)
    print("  \u2554" + "\u2550" * 48 + "\u2557")
    print("  \u2551  PATO'S WEBSOCKET VST - LIVE VISUALIZER          \u2551")
    print("  \u2560" + "\u2550" * 48 + "\u2563")
    print(RESET)
    print(fg(150, 150, 150) + "  Connecting to ws://localhost:%d ..." % port + RESET)
    print()
    time.sleep(1)

    s = connect_ws("127.0.0.1", port)
    print(fg(0, 255, 120) + "  Connected! Press Ctrl+C to quit." + RESET)
    time.sleep(1.5)

    frame_num = 0
    history = []
    last_bar = 0

    try:
        while True:
            data = recv_frame(s)
            if data is None:
                print(fg(255, 60, 60) + "\n  Connection closed." + RESET)
                break
            d = json.loads(data)

            current_bar = d["bar"]
            if current_bar != last_bar:
                last_bar = current_bar
                history = []

            if d["playing"]:
                frac = d["beat"] - int(d["beat"])
                intensity = int((1.0 - frac) * 8)
                history.append(max(1, intensity))
            else:
                history.append(0)

            if len(history) > 200:
                history = history[-200:]

            render(d, width, frame_num, history)
            frame_num += 1
            time.sleep(0.03)

    except KeyboardInterrupt:
        pass
    finally:
        sys.stdout.write(RESET + SHOW + CLS)
        sys.stdout.flush()
        s.close()
        print(fg(0, 180, 255) + "  Disconnected." + RESET)

main()
