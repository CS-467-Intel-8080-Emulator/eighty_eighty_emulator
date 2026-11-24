#include "shift_register.hpp"
#include "state/state.hpp"

void ShiftRegister::write_data(uint8_t value) {
    value_ = (uint16_t)((value << 8) | (value_ >> 8));
}

void ShiftRegister::write_offset(uint8_t value) {
    offset_ = value & 0x07;
}

uint8_t ShiftRegister::read() const {
    return (uint8_t)(value_ >> (8 - offset_));
}

void ShiftRegister::save_state(StateWriter& writer) const {
    writer.save_u16(value_);
    writer.save_u8(offset_);
}

void ShiftRegister::load_state(StateReader& reader) {
    value_ = reader.load_u16();
    offset_ = reader.load_u8();
}
