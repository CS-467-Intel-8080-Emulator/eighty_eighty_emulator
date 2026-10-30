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
    // IN ports
    static constexpr uint8_t PORT_SHIFT_READ = 3;

    // OUT ports
    static constexpr uint8_t PORT_SHIFT_OFFSET = 2;
    static constexpr uint8_t PORT_SHIFT_DATA = 4;

    ShiftRegister shift_;
};

#endif
