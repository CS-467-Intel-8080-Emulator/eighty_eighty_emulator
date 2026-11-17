// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). This document summarizes the GenAI usage that led to my code:
// https://drive.google.com/file/d/1SDeaM_7hkEwhuc5EPaecAapZURuSGOfO/view?usp=sharing

#ifndef SOUND_FILE_HPP
#define SOUND_FILE_HPP

#include <vector>

// A sound file a machine wants loaded
struct SoundFile {
    const char*        name;      // file name, e.g. "0.wav"
    int                id;        // which sound; the machine decides what it means
    std::vector<float> samples;   // starts empty, filled by read_sound_files()
};

#endif
