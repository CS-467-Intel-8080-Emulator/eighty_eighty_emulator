#ifndef INVADERS_SHIFT_REGISTER_HPP
#define INVADERS_SHIFT_REGISTER_HPP

#include <cstdint>

// Space Invaders hardware shift register, used to shift sprites since the
// 8080 has no barrel shifter. The machine's IOBus maps it to ports:
//   OUT 4: write_data
//   IN  3: read
class ShiftRegister {
public:
    // New byte becomes the high byte; the old high byte moves to the low byte
    void write_data(uint8_t value);

    // High byte of the 16-bit register
    uint8_t read() const;

private:
    uint16_t value_ = 0;
};

#endif
