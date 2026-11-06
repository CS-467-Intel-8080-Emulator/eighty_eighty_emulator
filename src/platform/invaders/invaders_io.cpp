#include "invaders_io.hpp"

uint8_t InvadersIO::in(uint8_t port) {
    switch (port) {
        case PORT_SHIFT_READ: return shift_.read();
        // TODO: ports 0-2 are player inputs and DIP switches (#41)
        default: return 0x00;
    }
}

void InvadersIO::out(uint8_t port, uint8_t value) {
    switch (port) {
        case PORT_SHIFT_OFFSET: shift_.write_offset(value); break;
        case PORT_SOUND_1: sound_.write_port3(value); break;
        case PORT_SHIFT_DATA: shift_.write_data(value); break;
        case PORT_SOUND_2: sound_.write_port5(value); break;
        // Port 6 is the watchdog, which an emulator can ignore
        default: break;
    }
}
