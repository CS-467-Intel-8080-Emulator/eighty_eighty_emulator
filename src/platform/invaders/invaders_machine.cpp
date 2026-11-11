// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). The accompanying transcripts document the GenAI interaction that
// led to my code.
// https://drive.google.com/drive/folders/1ju4Le0a1KcFW42RTQtTtEUDd8LGAbAAA?usp=drive_link

#include "invaders_machine.hpp"
#include "state/state.hpp"

InvadersMachine::InvadersMachine() {
    cpu_.set_io(&io_);
}

bool InvadersMachine::load_rom(const std::vector<uint8_t>& bytes, uint16_t offset) {
    return memory_.load_rom(bytes, offset);
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

std::vector<uint8_t> InvadersMachine::save_state() const {
    StateWriter writer = StateWriter();
    cpu_.save_state(writer);
    memory_.save_state(writer); // Saves RAM and VRAM only
    io_.save_state(writer);
    schedule_.save_state(writer);
    return writer.get_buffer();
}

bool InvadersMachine::load_state(const std::vector<uint8_t>& buffer) {
    if (buffer.size() != STATE_SIZE)
        return false;
    StateReader reader = StateReader();
    reader.set_buffer(buffer);
    cpu_.load_state(reader);
    memory_.load_state(reader);
    io_.load_state(reader);
    schedule_.load_state(reader);
    return true;
}
