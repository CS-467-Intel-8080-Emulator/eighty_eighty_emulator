#include "invaders_io.hpp"

uint8_t InvadersIO::in(uint8_t port) {
    switch (port) {
        case 3: return shift_.read();
        // TODO: ports 0-2 are player inputs and DIP switches (#41)
        default: return 0x00;
    }
}

void InvadersIO::out(uint8_t port, uint8_t value) {
    switch (port) {
        case 2: shift_.write_offset(value); break;
        case 4: shift_.write_data(value); break;
        // TODO: ports 3 and 5 trigger sounds (#48)
        // Port 6 is the watchdog, which an emulator can ignore
        default: break;
    }
}
