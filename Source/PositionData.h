#pragma once
#include <cstdint>

struct PositionData {
    double  timeInSeconds  = 0.0;
    int64_t timeInSamples = 0;
    double  ppq            = 0.0;
    double  bpm            = 120.0;
    int     barNumber      = 1;
    double  beatInBar      = 0.0;
    int     timeSigNum     = 4;
    int     timeSigDen     = 4;
    bool    isPlaying      = false;
    bool    isRecording    = false;
    bool    isLooping      = false;
};
