#include "input_ports.hpp"

namespace {
    constexpr uint8_t PORT0_FIXED = 0x0E;   // bits 1-3 tied high
    constexpr uint8_t PORT1_FIXED = 0x08;   // bit 3 tied high
    
    // port #2 DIP switch bits
    constexpr uint8_t DIP_SHIPS_MASK      = 0x03;
    constexpr uint8_t DIP_EXTRA_SHIP_1000 = 0x08;
    constexpr uint8_t DIP_COIN_INFO_OFF   = 0x80;

    // turns port bits on or off
    void set_bit(uint8_t& port, uint8_t mask, bool on) {
        if (on)
            port |= mask;
        else
            port &= ~mask;
    }
}

void InputPorts::set(Input input, bool pressed) {
    switch (input) {
        case Input::Coin:    set_bit(port1_, 0x01, pressed); break;
        case Input::P2Start: set_bit(port1_, 0x02, pressed); break;
        case Input::P1Start: set_bit(port1_, 0x04, pressed); break;
        case Input::P1Fire:  set_bit(port1_, 0x10, pressed); break;
        case Input::P1Left:  set_bit(port1_, 0x20, pressed); break;
        case Input::P1Right: set_bit(port1_, 0x40, pressed); break;

        case Input::Tilt:    set_bit(port2_, 0x04, pressed); break;
        case Input::P2Fire:  set_bit(port2_, 0x10, pressed); break;
        case Input::P2Left:  set_bit(port2_, 0x20, pressed); break;
        case Input::P2Right: set_bit(port2_, 0x40, pressed); break;
    }
}

bool InputPorts::set_dips(const DipSwitches& dips) {
    if (dips.ships < 3 || dips.ships > 6)
        return false;
    dips_ = dips;
    return true;
}

uint8_t InputPorts::read_port0() const {
    return PORT0_FIXED;
}

uint8_t InputPorts::read_port1() const {
    return port1_ | PORT1_FIXED;
}

uint8_t InputPorts::read_port2() const {
    uint8_t value = port2_;
    value |= (dips_.ships - 3) & DIP_SHIPS_MASK;
    if (dips_.extra_ship_at_1000)
        value |= DIP_EXTRA_SHIP_1000;
    if (!dips_.show_coin_info)
        value |= DIP_COIN_INFO_OFF;
    return value;
}
