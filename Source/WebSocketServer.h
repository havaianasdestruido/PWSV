#pragma once
#include <juce_core/juce_core.h>
#include "PositionData.h"
#include <vector>
#include <string>
#include <atomic>
#include <chrono>

// fd_set is a sockets type. This header does not pull in the platform socket
// headers (on Windows juce_core only brings in windows.h + winsock.h via
// WIN32_LEAN_AND_MEAN, so fd_set is not declared), yet it is part of a private
// member signature below. Declare it here so the header stays self-contained:
// MSVC otherwise reports error C2061: syntax error: identifier 'fd_set'.
#if defined(_WIN32)
struct fd_set;
#else
#include <sys/select.h>
#endif

class WebSocketServer : public juce::Thread {
public:
    WebSocketServer();
    ~WebSocketServer() override;

    bool start(int port);
    void stop();
    void setPort(int port);

    void updatePosition(const PositionData& data);
    void setMode(int mode);
    void setRate(float hz);
    void setBeatDivision(int index);
    void setBpm(double bpm);

    int  getConnectedClientCount() const;
    bool isRunning() const;
    juce::String getLastError() const;

    static constexpr int NUM_BEAT_DIVISIONS = 10;
    static const char*   beatDivisionLabels[NUM_BEAT_DIVISIONS];
    static const double  beatDivisionPpq[NUM_BEAT_DIVISIONS];

private:
    void run() override;

    void acceptNewClients();
    void readFromClients(fd_set& readSet);
    bool performHandshake(int sock);
    void sendToAll(const std::string& payload);
    void disconnectClient(size_t index);

    bool tryRebind();

    bool shouldSend();
    void afterSend(const PositionData& p);
    const std::string& buildJson(const PositionData& p);

    PositionData getPosition() const;

    int listenSocket_ = -1;
    int port_         = 8080;
    std::atomic<int> targetPort_{ -1 };
    std::vector<int> clients_;
    std::vector<uint8_t> sendBuffer_;
    std::string jsonBuffer_; // Pre-allocated buffer for sendToAll to avoid per-broadcast allocations

    mutable juce::SpinLock posLock_;
    PositionData pos_;

    std::atomic<int>   mode_    { 0 };
    std::atomic<float> rate_    { 10.0f };
    std::atomic<int>   beatDiv_ { 2 };
    std::atomic<double> bpm_    { 120.0 };

    std::atomic<int>  clientCount_ { 0 };
    std::atomic<bool> running_     { false };
    juce::String lastError_;

    std::atomic<double> currentPpq_ { 0.0 };
    std::atomic<bool>   isPlaying_  { false };

    double lastSentPpq_ = -1.0;
    bool   wasPlaying_  = false;
    std::chrono::steady_clock::time_point lastSendTime_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebSocketServer)
};
