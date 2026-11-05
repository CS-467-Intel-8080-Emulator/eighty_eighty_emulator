#ifndef MACHINE_HPP
#define MACHINE_HPP

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
};

#endif
