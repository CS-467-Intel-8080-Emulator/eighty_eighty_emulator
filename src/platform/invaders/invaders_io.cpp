#include "invaders_io.hpp"
#include "state/state.hpp"

uint8_t InvadersIO::in(uint8_t port) {
    switch (port) {
        case PORT_INPUTS_0: return inputs_.read_port0();
        case PORT_INPUTS_1: return inputs_.read_port1();
        case PORT_INPUTS_2: return inputs_.read_port2();
        case PORT_SHIFT_READ: return shift_.read();
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

void InvadersIO::save_state(StateWriter& writer) const {
    shift_.save_state(writer);
}

void InvadersIO::load_state(StateReader& reader) {
    shift_.load_state(reader);
}
