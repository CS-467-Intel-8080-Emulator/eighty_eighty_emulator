#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <algorithm>
#include <cstdint>


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
};

#endif
