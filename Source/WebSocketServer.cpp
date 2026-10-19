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

// ── setters (called from any thread) ─────────────────────────

void WebSocketServer::updatePosition(const PositionData& d) {
    // try_lock prevents priority inversion / blocking in the audio thread.
    // If the lock is held by the server thread, we simply skip this update.
    // Since processBlock runs much faster than the websocket send rate,
    // skipping an occasional update is perfectly fine and avoids audio dropouts.
    if (posLock_.try_lock()) {
        pos_ = d;
        posLock_.unlock();
    }
}

void WebSocketServer::setMode(int m)           { mode_.store(m); }
void WebSocketServer::setRate(float hz)        { rate_.store(hz); }
void WebSocketServer::setBeatDivision(int idx) { beatDiv_.store(idx); }
void WebSocketServer::setBpm(double b)         { bpm_.store(b); }

int  WebSocketServer::getConnectedClientCount() const { return clientCount_.load(); }
bool WebSocketServer::isRunning()              const { return running_.load(); }
juce::String WebSocketServer::getLastError()   const { return lastError_; }

PositionData WebSocketServer::getPosition() const {
    std::lock_guard<std::mutex> lk(posLock_);
    return pos_;
}

// ── main thread loop ─────────────────────────────────────────

void WebSocketServer::run() {
    initSockets();

    listenSocket_ = static_cast<int>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (listenSocket_ == -1) { lastError_ = "socket() failed"; return; }

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
        closesocket(listenSocket_); listenSocket_ = -1; return;
    }

    if (listen(listenSocket_, 4) == SOCKET_ERROR) {
        lastError_ = "listen() failed";
        closesocket(listenSocket_); listenSocket_ = -1; return;
    }

    setNonBlocking(listenSocket_);
    running_    = true;
    lastSendTime_ = std::chrono::steady_clock::now();

    while (!threadShouldExit()) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listenSocket_, &readSet);
        SOCKET maxFd = listenSocket_;

        for (auto s : clients_) {
            FD_SET(s, &readSet);
            if (s > maxFd) maxFd = s;
        }

        timeval tv{ 0, 1000 };
        select(
#ifdef _WIN32
            0,
#else
            static_cast<int>(maxFd + 1),
#endif
            &readSet, nullptr, nullptr, &tv);

        if (FD_ISSET(listenSocket_, &readSet))
            acceptNewClients();

        readFromClients(readSet);

        if (!clients_.empty()) {
            PositionData p = getPosition();
            if (shouldSend(p)) {
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

void WebSocketServer::sendToAll(const juce::String& payload) {
    if (clients_.empty()) return;
    auto utf8 = payload.toRawUTF8();
    auto len = static_cast<size_t>(payload.getNumBytesAsUTF8());
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

bool WebSocketServer::shouldSend(const PositionData& p) {
    int m = mode_.load();

    if (m == 1) {
        bool justStarted = p.isPlaying && !wasPlaying_;
        wasPlaying_ = p.isPlaying;
        if (!p.isPlaying) return false;
        if (justStarted) return true;

        double div = beatDivisionPpq[beatDiv_.load()];
        int cur = static_cast<int>(std::floor(p.ppq / div));
        int lst = static_cast<int>(std::floor(lastSentPpq_ / div));
        return cur != lst;
    }

    wasPlaying_ = p.isPlaying;
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

static juce::String dbl(double v, int prec = 3) {
    return juce::String(v, prec);
}

juce::String WebSocketServer::buildJson(const PositionData& p) {
    juce::String json;
    // Pre-allocate enough memory to hold maximum possible notes without reallocations.
    // Base JSON is ~200 chars. Each note is ~30 chars. Max 128 notes. Total ~4000 chars.
    json.preallocateBytes(4096);

    json << "{"
         << "\"protocol\":2,"
         << "\"time_sec\":"    << dbl(p.timeInSeconds)   << ","
         << "\"time_samples\":" << juce::String(p.timeInSamples) << ","
         << "\"ppq\":"         << dbl(p.ppq)             << ","
         << "\"bpm\":"         << dbl(p.bpm, 1)          << ","
         << "\"bar\":"         << juce::String(p.barNumber) << ","
         << "\"beat\":"        << dbl(p.beatInBar, 2)    << ","
         << "\"time_sig\":["   << juce::String(p.timeSigNum) << "," << juce::String(p.timeSigDen) << "],"
         << "\"playing\":"     << (p.isPlaying   ? "true" : "false") << ","
         << "\"recording\":"   << (p.isRecording ? "true" : "false") << ","
         << "\"looping\":"     << (p.isLooping   ? "true" : "false") << ","
         << "\"notes\":[";

    for (int i = 0; i < p.activeNoteCount; ++i) {
        if (i > 0) json << ",";
        json << "{"
             << "\"note\":" << juce::String(p.activeNoteNumbers[i]) << ","
             << "\"vel\":" << juce::String(p.activeNoteVelocities[i]) << ","
             << "\"ch\":" << juce::String(p.activeNoteChannels[i] + 1)
             << "}";
    }
    json << "]}";

    return json;
}
