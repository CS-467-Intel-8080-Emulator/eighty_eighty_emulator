#ifndef INVADERS_IO_HPP
#define INVADERS_IO_HPP

#include <cstdint>

#include "core/io_bus.hpp"
#include "shift_register.hpp"

// Space Invaders port map: routes the CPU's IN and OUT to the board's devices
class InvadersIO : public IOBus {
public:
    uint8_t in(uint8_t port) override;
    void out(uint8_t port, uint8_t value) override;

private:
    ShiftRegister shift_;
};

#endif
