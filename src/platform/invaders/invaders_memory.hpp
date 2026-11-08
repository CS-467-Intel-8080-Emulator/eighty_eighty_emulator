// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). The accompanying transcripts document the GenAI interaction that
// led to my code.
// https://drive.google.com/drive/folders/1ju4Le0a1KcFW42RTQtTtEUDd8LGAbAAA?usp=drive_link

#ifndef INVADERS_MEMORY_HPP
#define INVADERS_MEMORY_HPP

#include <array>
#include <cstdint>
#include <vector>

#include "core/memory_bus.hpp"

/*
Space Invaders hardware contains 16 KB of memory (0x0000-0x3FFF) which requires
only 14 of the CPU's 16 addressing bits. Those 16KB of memory are mapped like so:

    0x0000-0x1FFF       8K ROM (ignores writes after initial load)
    0x2000-0x23FF       1K work RAM
    0x2400-0x3FFF       7k video RAM
                        = 16K total memory

For this particular hardware platform, the CPU's address pin 15 is unconnected;
pin 14 is technically connected to allow addressing for an additional ROM slot
at 0x4000-0x5FFF; however this slot is not used for Space Invaders, so in this
case for anything above 0x3FFF we just mask off those top two bits.

See MAME: https://raw.githubusercontent.com/mamedev/mame/master/src/mame/midw8080/mw8080bw.cpp

    0x4000-0xFFFF       ROM/RAM "mirrors" (masked off with 0x3FFF)
*/
class InvadersMemory : public MemoryBus {
public:
    static constexpr uint16_t ROM_SIZE = 0x2000;
    static constexpr uint16_t RAM_START = 0x2000;
    static constexpr uint16_t ADDR_MASK = 0x3FFF;

    InvadersMemory();

    uint8_t read(uint16_t addr) const override;
    void write(uint16_t addr, uint8_t value) override;

    // For initial ROM load; bypasses write()'s read-only guard on 0x0000-0x1FFF
    // Returns false and changes nothing if the bytes don't fit in ROM block
    bool load_rom(const std::vector<uint8_t>& bytes, uint16_t offset = 0);

private:
    std::array<uint8_t, ADDR_MASK + 1> mem_;
};

#endif
