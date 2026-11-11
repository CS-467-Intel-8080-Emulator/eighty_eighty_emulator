#include "invaders_io.hpp"
#include "state/state.hpp"

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
    case PORT_SHIFT_DATA: shift_.write_data(value); break;
        // TODO: ports 3 and 5 trigger sounds (#48)
        // Port 6 is the watchdog, which an emulator can ignore
    default: break;
    }
}

void InvadersIO::save_state(StateWriter& writer) const {
    shift_.save_state(writer);
}

void InvadersIO::load_state(StateReader& reader) {
    shift_.load_state(reader);
}
