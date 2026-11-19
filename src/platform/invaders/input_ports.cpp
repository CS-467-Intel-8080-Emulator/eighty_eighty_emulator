#include "input_ports.hpp"

namespace {
    constexpr uint8_t PORT0_FIXED = 0x0E;   // bits 1-3 tied high
    constexpr uint8_t PORT1_FIXED = 0x08;   // bit 3 tied high

    constexpr uint8_t PORT1_COIN    = 0x01;
    constexpr uint8_t PORT1_P2_START = 0x02;
    constexpr uint8_t PORT1_P1_START = 0x04;
    constexpr uint8_t PORT1_P1_FIRE  = 0x10;
    constexpr uint8_t PORT1_P1_LEFT  = 0x20;
    constexpr uint8_t PORT1_P1_RIGHT = 0x40;

    constexpr uint8_t PORT2_TILT     = 0x04;
    constexpr uint8_t PORT2_P2_FIRE  = 0x10;
    constexpr uint8_t PORT2_P2_LEFT  = 0x20;
    constexpr uint8_t PORT2_P2_RIGHT = 0x40;

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
        case Input::Coin:    set_bit(port1_, PORT1_COIN, pressed); break;
        case Input::P2Start: set_bit(port1_, PORT1_P2_START, pressed); break;
        case Input::P1Start: set_bit(port1_, PORT1_P1_START, pressed); break;
        case Input::P1Fire:  set_bit(port1_, PORT1_P1_FIRE, pressed); break;
        case Input::P1Left:  set_bit(port1_, PORT1_P1_LEFT, pressed); break;
        case Input::P1Right: set_bit(port1_, PORT1_P1_RIGHT, pressed); break;

        case Input::Tilt:    set_bit(port2_, PORT2_TILT, pressed); break;
        case Input::P2Fire:  set_bit(port2_, PORT2_P2_FIRE, pressed); break;
        case Input::P2Left:  set_bit(port2_, PORT2_P2_LEFT, pressed); break;
        case Input::P2Right: set_bit(port2_, PORT2_P2_RIGHT, pressed); break;
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
