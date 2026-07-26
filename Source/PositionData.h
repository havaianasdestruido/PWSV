#pragma once
#include <cstdint>
#include <algorithm>

struct NoteEvent {
    int note     = 0;
    int velocity = 0;
    int channel  = 0;
};

struct PositionData {
    double  timeInSeconds  = 0.0;
    int64_t timeInSamples  = 0;
    double  ppq            = 0.0;
    double  bpm            = 120.0;
    int     barNumber      = 1;
    double  beatInBar      = 0.0;
    int     timeSigNum     = 4;
    int     timeSigDen     = 4;
    bool    isPlaying      = false;
    bool    isRecording    = false;
    bool    isLooping      = false;

    static constexpr int MAX_NOTES = 128;
    int  activeNoteNumbers[MAX_NOTES] = {};
    int  activeNoteVelocities[MAX_NOTES] = {};
    int  activeNoteChannels[MAX_NOTES] = {};
    int  activeNoteCount = 0;
};
