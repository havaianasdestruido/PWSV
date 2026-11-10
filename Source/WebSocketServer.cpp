#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef int socklen_t;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/select.h>
  typedef int SOCKET;
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR   (-1)
  #define closesocket    ::close
#endif

#include "WebSocketServer.h"
#include "Sha1.h"
#include "Base64.h"
#include <cmath>
#include <algorithm>
#include <string>
#include <charconv>
#include <type_traits>

static void setNonBlocking(SOCKET s) {
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
#else
    int flags = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);
#endif
}

static void initSockets() {
#ifdef _WIN32
    static bool done = false;
    if (!done) { WSADATA d; WSAStartup(MAKEWORD(2, 2), &d); done = true; }
#endif
}

static void cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

// ── beat-division table ──────────────────────────────────────

const char* WebSocketServer::beatDivisionLabels[WebSocketServer::NUM_BEAT_DIVISIONS] = {
    "1/1", "1/2", "1/4", "1/8", "1/16", "1/32",
    "3/4", "3/8", "3/16", "3/32"
};

const double WebSocketServer::beatDivisionPpq[WebSocketServer::NUM_BEAT_DIVISIONS] = {
    4.0,         // 1/1  whole note
    2.0,         // 1/2  half note
    1.0,         // 1/4  quarter note
    0.5,         // 1/8  eighth note
    0.25,        // 1/16 sixteenth note
    0.125,       // 1/32 thirty-second note
    2.0 / 3.0,   // 3/4  quarter-note triplet
    1.0 / 3.0,   // 3/8  eighth-note triplet
    0.5 / 3.0,   // 3/16 sixteenth-note triplet
    0.25 / 3.0   // 3/32 thirty-second-note triplet
};

// ── lifecycle ────────────────────────────────────────────────

WebSocketServer::WebSocketServer()
    : Thread("PatoWS_Server") {}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start(int port) {
    stop();
    port_ = port;
    lastError_.clear();
    startThread(Thread::Priority::normal);
    return true;
}

void WebSocketServer::stop() {
    if (isThreadRunning()) {
        signalThreadShouldExit();
        stopThread(2000);
    }
    for (auto s : clients_) closesocket(s);
    clients_.clear();
    clientCount_ = 0;
    if (listenSocket_ != -1) { closesocket(listenSocket_); listenSocket_ = -1; }
    running_ = false;
}

void WebSocketServer::setPort(int port) {
    targetPort_.store(port, std::memory_order_relaxed);
}

// ── setters (called from any thread) ─────────────────────────

void WebSocketServer::updatePosition(const PositionData& d) {
    // Fast path: update atomics for the server thread to check without locking
    if (currentPpq_.load(std::memory_order_relaxed) != d.ppq)
        currentPpq_.store(d.ppq, std::memory_order_relaxed);
    if (isPlaying_.load(std::memory_order_relaxed) != d.isPlaying)
        isPlaying_.store(d.isPlaying, std::memory_order_relaxed);

    // tryEnter prevents priority inversion / blocking in the audio thread.
    // If the lock is held by the server thread, we simply skip this update.
    // Since processBlock runs much faster than the websocket send rate,
    // skipping an occasional update is perfectly fine and avoids audio dropouts.
    if (posLock_.tryEnter()) {
        pos_ = d;
        posLock_.exit();
    }
}

void WebSocketServer::setMode(int m) {
    if (mode_.load(std::memory_order_relaxed) != m)
        mode_.store(m, std::memory_order_relaxed);
}
void WebSocketServer::setRate(float hz) {
    if (rate_.load(std::memory_order_relaxed) != hz)
        rate_.store(hz, std::memory_order_relaxed);
}
void WebSocketServer::setBeatDivision(int idx) {
    if (beatDiv_.load(std::memory_order_relaxed) != idx)
        beatDiv_.store(idx, std::memory_order_relaxed);
}
void WebSocketServer::setBpm(double b) {
    if (bpm_.load(std::memory_order_relaxed) != b)
        bpm_.store(b, std::memory_order_relaxed);
}

int  WebSocketServer::getConnectedClientCount() const { return clientCount_.load(std::memory_order_relaxed); }
bool WebSocketServer::isRunning()              const { return running_.load(); }
juce::String WebSocketServer::getLastError()   const { return lastError_; }

PositionData WebSocketServer::getPosition() const {
    juce::SpinLock::ScopedLockType lk(posLock_);
    return pos_;
}

// ── main thread loop ─────────────────────────────────────────

bool WebSocketServer::tryRebind() {
    if (listenSocket_ != -1) { closesocket(listenSocket_); listenSocket_ = -1; }

    listenSocket_ = static_cast<int>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (listenSocket_ == -1) { lastError_ = "socket() failed"; return false; }

    int opt = 1;
    setsockopt(listenSocket_, SOL_SOCKET, SO_REUSEADDR,
#ifdef _WIN32
        (const char*)&opt,
#else
        &opt,
#endif
        sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(static_cast<uint16_t>(port_));

    if (bind(listenSocket_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        lastError_ = "bind() failed on port " + juce::String(port_);
        closesocket(listenSocket_); listenSocket_ = -1; return false;
    }

    if (listen(listenSocket_, 4) == SOCKET_ERROR) {
        lastError_ = "listen() failed";
        closesocket(listenSocket_); listenSocket_ = -1; return false;
    }

    setNonBlocking(listenSocket_);
    return true;
}

void WebSocketServer::run() {
    initSockets();

    if (tryRebind()) {
        running_ = true;
    } else {
        running_ = false;
    }

    lastSendTime_ = std::chrono::steady_clock::now();

    while (!threadShouldExit()) {
        int target = targetPort_.exchange(-1, std::memory_order_relaxed);
        if (target != -1) {
            port_ = target;

            for (auto s : clients_) closesocket(s);
            clients_.clear();
            clientCount_.store(0, std::memory_order_relaxed);

            if (!tryRebind()) {
                running_ = false;
                // Wait for the next port change request or shutdown
                juce::Thread::sleep(100);
                continue;
            }
            running_ = true;
        }

        fd_set readSet;
        FD_ZERO(&readSet);

        SOCKET maxFd = -1;
        if (listenSocket_ != -1) {
            FD_SET(listenSocket_, &readSet);
            maxFd = listenSocket_;
        }

        for (auto s : clients_) {
            FD_SET(s, &readSet);
            if (s > maxFd) maxFd = s;
        }

        timeval tv{ 0, 1000 };

        if (maxFd != -1) {
            select(
#ifdef _WIN32
                0,
#else
                static_cast<int>(maxFd + 1),
#endif
                &readSet, nullptr, nullptr, &tv);

            if (listenSocket_ != -1 && FD_ISSET(listenSocket_, &readSet))
                acceptNewClients();

            readFromClients(readSet);
        } else {
            // No valid sockets to listen on, avoid CPU spinning
            juce::Thread::sleep(100);
        }

        if (!clients_.empty()) {
            if (shouldSend()) {
                PositionData p = getPosition();
                sendToAll(buildJson(p));
                afterSend(p);
            }
        }
    }

    for (auto s : clients_) closesocket(s);
    clients_.clear();
    clientCount_ = 0;
    closesocket(listenSocket_); listenSocket_ = -1;
    running_ = false;
}

// ── accept / read / handshake ────────────────────────────────

void WebSocketServer::acceptNewClients() {
    for (;;) {
        sockaddr_in cAddr{};
        socklen_t cLen = sizeof(cAddr);
        int csock = static_cast<int>(accept(listenSocket_, (sockaddr*)&cAddr, &cLen));
        if (csock == -1) break;
        if (clients_.size() >= 8) { closesocket(csock); continue; }
#ifdef _WIN32
        u_long block = 0;
        ioctlsocket(csock, FIONBIO, &block);
#endif
        if (performHandshake(csock)) {
            // Disable Nagle's Algorithm to prevent ~200ms latency in real-time WebSocket traffic
            int opt = 1;
            setsockopt(csock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt));

            setNonBlocking(csock);
            clients_.push_back(csock);
            clientCount_.store(static_cast<int>(clients_.size()));
        } else {
            closesocket(csock);
        }
    }
}

bool WebSocketServer::performHandshake(int sock) {
    char buf[4096];
    int n = recv(sock, buf, sizeof(buf) - 1, 0);
    if (n <= 0) return false;
    buf[n] = '\0';

    juce::String request(buf);
    juce::String keyHeader("Sec-WebSocket-Key: ");
    int keyPos = request.indexOf(keyHeader);
    if (keyPos < 0) return false;

    keyPos += keyHeader.length();
    int keyEnd = request.indexOf(keyPos, "\r\n");
    if (keyEnd < 0) return false;

    juce::String key = request.substring(keyPos, keyEnd);
    juce::String magic("258EAFA5-E914-47DA-95CA-5AB5DC76B97E");
    juce::String concat = key + magic;

    Sha1 sha;
    sha.update(concat.toRawUTF8(), static_cast<size_t>(concat.length()));
    uint8_t hash[20];
    sha.finalize(hash);

    std::string acceptKey = Base64::encode(hash, 20);

    juce::String response =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: " + juce::String(acceptKey.c_str()) + "\r\n\r\n";

    send(sock, response.toRawUTF8(), static_cast<int>(response.length()), 0);
    return true;
}

void WebSocketServer::readFromClients(fd_set& readSet) {
    char tmp[128];
    for (size_t i = clients_.size(); i-- > 0;) {
        if (FD_ISSET(clients_[i], &readSet)) {
            int n = recv(clients_[i], tmp, sizeof(tmp), 0);
            if (n == 0 || n == -1) {
#ifdef _WIN32
                int err = WSAGetLastError();
                if (n == -1 && (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS))
                    continue;
#else
                if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    continue;
#endif
                disconnectClient(i);
            }
        }
    }
}

void WebSocketServer::disconnectClient(size_t idx) {
    closesocket(clients_[idx]);
    clients_.erase(clients_.begin() + static_cast<ptrdiff_t>(idx));
    clientCount_.store(static_cast<int>(clients_.size()));
}

// ── send ─────────────────────────────────────────────────────

void WebSocketServer::sendToAll(const std::string& payload) {
    if (clients_.empty()) return;
    auto utf8 = reinterpret_cast<const uint8_t*>(payload.data());
    auto len = payload.size();
    sendBuffer_.clear();
    sendBuffer_.reserve(10 + len);
    sendBuffer_.push_back(0x81);
    if (len < 126) {
        sendBuffer_.push_back(static_cast<uint8_t>(len));
    } else if (len < 65536) {
        sendBuffer_.push_back(126);
        sendBuffer_.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        sendBuffer_.push_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        sendBuffer_.push_back(127);
        for (int i = 7; i >= 0; --i)
            sendBuffer_.push_back(static_cast<uint8_t>((len >> (i * 8)) & 0xFF));
    }
    sendBuffer_.insert(sendBuffer_.end(), utf8, utf8 + len);
    for (size_t i = clients_.size(); i-- > 0;) {
        send(clients_[i], (const char*)sendBuffer_.data(), static_cast<int>(sendBuffer_.size()), 0);
    }
}

// ── rate / beat logic ────────────────────────────────────────

bool WebSocketServer::shouldSend() {
    int m = mode_.load();
    bool playing = isPlaying_.load();
    double ppq = currentPpq_.load();

    if (m == 1) {
        bool justStarted = playing && !wasPlaying_;
        wasPlaying_ = playing;
        if (!playing) return false;
        if (justStarted) return true;

        double div = beatDivisionPpq[beatDiv_.load()];
        int cur = static_cast<int>(std::floor(ppq / div));
        int lst = static_cast<int>(std::floor(lastSentPpq_ / div));
        return cur != lst;
    }

    wasPlaying_ = playing;
    auto now    = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - lastSendTime_).count();
    double hz = static_cast<double>(rate_.load());
    if (hz <= 0.0) hz = 1.0;
    return elapsed >= (1.0 / hz);
}

void WebSocketServer::afterSend(const PositionData& p) {
    lastSentPpq_  = p.ppq;
    lastSendTime_ = std::chrono::steady_clock::now();
}

// ── JSON ─────────────────────────────────────────────────────

struct DblFmt {
    char buf[32];
    size_t len;
    DblFmt(double v, int prec = 3) {
        auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v, std::chars_format::fixed, prec);
        if (ec == std::errc()) {
            len = static_cast<size_t>(ptr - buf);
        } else {
            buf[0] = '0';
            len = 1;
        }
    }
};

static std::string& operator<<(std::string& s, const char* str) {
    s.append(str);
    return s;
}

static std::string& operator<<(std::string& s, const DblFmt& fmt) {
    s.append(fmt.buf, fmt.len);
    return s;
}

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
static std::string& operator<<(std::string& s, T v) {
    char buf[32];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v);
    if (ec == std::errc()) {
        s.append(buf, ptr - buf);
    }
    return s;
}

const std::string& WebSocketServer::buildJson(const PositionData& p) {
    if (jsonBuffer_.capacity() < 4096) {
        jsonBuffer_.reserve(4096);
    }
    jsonBuffer_.clear();

    jsonBuffer_ << "{"
         << "\"protocol\":2,"
         << "\"time_sec\":"    << DblFmt(p.timeInSeconds)   << ","
         << "\"time_samples\":" << p.timeInSamples       << ","
         << "\"ppq\":"         << DblFmt(p.ppq)             << ","
         << "\"bpm\":"         << DblFmt(p.bpm, 1)          << ","
         << "\"bar\":"         << p.barNumber            << ","
         << "\"beat\":"        << DblFmt(p.beatInBar, 2)    << ","
         << "\"time_sig\":["   << p.timeSigNum           << "," << p.timeSigDen << "],"
         << "\"playing\":"     << (p.isPlaying   ? "true" : "false") << ","
         << "\"recording\":"   << (p.isRecording ? "true" : "false") << ","
         << "\"looping\":"     << (p.isLooping   ? "true" : "false") << ","
         << "\"notes\":[";

    for (int i = 0; i < p.activeNoteCount; ++i) {
        if (i > 0) jsonBuffer_ << ",";
        jsonBuffer_ << "{"
             << "\"note\":" << p.activeNoteNumbers[i]   << ","
             << "\"vel\":"  << p.activeNoteVelocities[i] << ","
             << "\"ch\":"   << (p.activeNoteChannels[i] + 1)
             << "}";
    }
    jsonBuffer_ << "]}";

    return jsonBuffer_;
}
