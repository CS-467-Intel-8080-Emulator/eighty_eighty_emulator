#include "shift_register.hpp"

void ShiftRegister::write_data(uint8_t value) {
    value_ = (uint16_t)((value << 8) | (value_ >> 8));
}

void ShiftRegister::write_offset(uint8_t value) {
    offset_ = value & 0x07;
}

uint8_t ShiftRegister::read() const {
    return (uint8_t)(value_ >> (8 - offset_));
}
