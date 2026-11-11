// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). The accompanying transcripts document the GenAI interaction that
// led to my code.
// https://drive.google.com/drive/folders/1ju4Le0a1KcFW42RTQtTtEUDd8LGAbAAA?usp=drive_link

#include "invaders_memory.hpp"
#include "tools/state.hpp"

#include <cstddef>

InvadersMemory::InvadersMemory() {
    mem_.fill(0);
}

uint8_t InvadersMemory::read(uint16_t addr) const {
    return mem_[addr & ADDR_MASK];
}

void InvadersMemory::write(uint16_t addr, uint8_t value) {
    addr &= ADDR_MASK;
    if (addr < RAM_START)
        return; // Read-only guard for ROM block
    mem_[addr] = value;
}

bool InvadersMemory::load_rom(const std::vector<uint8_t>& bytes, uint16_t offset) {
    if (offset + bytes.size() > ROM_SIZE)
        return false; // Doesn't fit in ROM block
    for (std::size_t i = 0; i < bytes.size(); i++) {
        mem_[offset + i] = bytes[i];
    }
    return true;
}

void InvadersMemory::save_state(StateWriter& writer) const {
    for (std::size_t i = RAM_START; i < mem_.size(); i++) {
        writer.save_u8(mem_[i]);
    }
}

void InvadersMemory::load_state(StateReader& reader) {
    for (std::size_t i = RAM_START; i < mem_.size(); i++) {
        mem_[i] = reader.load_u8();
    }
}
