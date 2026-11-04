#include "invaders_machine.hpp"

InvadersMachine::InvadersMachine() {
    cpu_.set_io(&io_);
}

// TODO: Update with proper rom loading using Space Invaders memory
void InvadersMachine::load_rom(uint16_t address, const std::vector<uint8_t>& bytes) {
    for (std::size_t i = 0; i < bytes.size(); i++) {
        memory_.write(static_cast<uint16_t>(address + i), bytes[i]);
    }
}

void InvadersMachine::run_frame() {
    bool frame_done = false;

    while (!frame_done) {
        uint32_t cycles = 0;

        // Fire pending interrupts + record cycles
        if (std::optional<uint8_t> rst = schedule_.pending_rst()) {
            if (uint32_t rst_cycles = cpu_.interrupt(memory_, *rst)) {
                schedule_.clear_pending();
                cycles += rst_cycles;
            }
        }

        // Execute next instruction + record cycles
        cycles += cpu_.step(memory_);

        // Accumulate recorded cycles into the interrupt scheduler
        frame_done = schedule_.add_cycles(cycles);
    }

    video_.update(memory_);
}
