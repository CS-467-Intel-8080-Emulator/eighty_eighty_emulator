// The rom_reader files were generated with the help of Claude Code.
// This document summarizes the GenAI usage that led to this code:
// https://drive.google.com/file/d/1kHRbPPfuhywTTaPXBz7GwhoZSb2PxOZz/view?usp=drive_link

#ifndef ROM_READER_HPP
#define ROM_READER_HPP

#include <cstdint>
#include <string>
#include <vector>

// Description of a single ROM file for a game - games may consist of multiple ROM files.
struct RomFile {
    const char*          name;      // file name, e.g. "invaders.h"
    uint32_t             size;      // file size in bytes
    uint16_t             address;   // where the machine stores file in memory
    std::vector<uint8_t> bytes;     // file contents - starts empty, filled by read_rom_files()
};

// Reads and validates ROM files in folder.
std::vector<RomFile> read_rom_files(const std::string& folder, const std::vector<RomFile>& files);

#endif
