// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). The accompanying transcripts document the GenAI interaction that
// led to my code.
// https://drive.google.com/drive/folders/1ju4Le0a1KcFW42RTQtTtEUDd8LGAbAAA?usp=drive_link


#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <algorithm>
#include <cstdint>
#include <vector>

#include "platform/sound_file.hpp"


class Machine {
public:
    virtual ~Machine() = default;

    virtual const char* title() const = 0;
    virtual int display_width() const = 0;
    virtual int display_height() const = 0;
    virtual double frames_per_second() const = 0;

    virtual void run_frame() = 0;
    virtual const uint32_t* pixels() const = 0;

    // Sample rate 0 means no audio
    virtual int audio_sample_rate() const { return 0; }
    virtual void render_audio(float* out, int count) { std::fill(out, out + count, 0.0f); }

    // Sound files this machine wants, and where each one goes once read
    virtual std::vector<SoundFile> sound_files() const { return {}; }
    virtual bool load_sound(int /*id*/, std::vector<float> /*samples*/) { return false; }
};

#endif
