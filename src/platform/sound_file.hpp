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
