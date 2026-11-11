// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). The accompanying transcripts document the GenAI interaction that
// led to my code.
// https://drive.google.com/drive/folders/1ju4Le0a1KcFW42RTQtTtEUDd8LGAbAAA?usp=drive_link


#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <cstdint>

enum class Input {
    Coin,
    P1Start,
    P2Start,
    P1Left,
    P1Right,
    P1Fire,
    P2Left,
    P2Right,
    P2Fire,
    Tilt,
};

class Machine {
public:
    virtual ~Machine() = default;

    virtual const char* title() const = 0;
    virtual int display_width() const = 0;
    virtual int display_height() const = 0;
    virtual double frames_per_second() const = 0;

    virtual void run_frame() = 0;
    virtual const uint32_t* pixels() const = 0;

    // press or release controls
    virtual void set_input(Input /*input*/, bool /*pressed*/) {}
};

#endif
