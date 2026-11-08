#ifndef INVADERS_ROM_HPP
#define INVADERS_ROM_HPP

#include <vector>

#include "rom/rom_reader.hpp"

// Space Invaders ROM: four 2 KB files, stored in order at 0x0000-0x1FFF.
// Players supply these files themselves (they're copyrighted).
inline const std::vector<RomFile> INVADERS_ROM_FILES = {
    {"invaders.h", 0x800, 0x0000},
    {"invaders.g", 0x800, 0x0800},
    {"invaders.f", 0x800, 0x1000},
    {"invaders.e", 0x800, 0x1800},
};

#endif
