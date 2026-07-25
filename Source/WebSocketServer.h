#pragma once
#include <juce_core/juce_core.h>
#include "PositionData.h"
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>

class WebSocketServer : public juce::Thread {
public:
    WebSocketServer();
    ~WebSocketServer() override;

    bool start(int port);
    void stop();

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
    void readFromClients();
    bool performHandshake(int sock);
    void sendTextFrame(int sock, const juce::String& payload);
    void sendToAll(const juce::String& payload);
    void disconnectClient(size_t index);

    bool shouldSend();
    void afterSend();
    juce::String buildJson();

    PositionData getPosition() const;

    int listenSocket_ = -1;
    int port_         = 8080;
    std::vector<int> clients_;

    mutable std::mutex posLock_;
    PositionData pos_;

    std::atomic<int>   mode_    { 0 };
    std::atomic<float> rate_    { 10.0f };
    std::atomic<int>   beatDiv_ { 2 };
    std::atomic<double> bpm_    { 120.0 };

    std::atomic<int>  clientCount_ { 0 };
    std::atomic<bool> running_     { false };
    juce::String lastError_;

    double lastSentPpq_ = -1.0;
    bool   wasPlaying_  = false;
    std::chrono::steady_clock::time_point lastSendTime_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebSocketServer)
};
