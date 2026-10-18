#ifndef IO_BUS_HPP
#define IO_BUS_HPP

#include <cstdint>

// Port I/O for IN/OUT. The machine (e.g. Space Invaders) implements this with
// its own devices: input ports, the shift register, sound triggers.
class IOBus {
public:
    virtual ~IOBus() = default;

    // IN port: value placed in A
    virtual uint8_t in(uint8_t port) = 0;

    // OUT port: A is sent to the port
    virtual void out(uint8_t port, uint8_t value) = 0;
};

#endif
