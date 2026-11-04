#ifndef ROM_LOADER_HPP
#define ROM_LOADER_HPP

#include <cstdint>
#include <string>
#include <vector>

// Description of a single ROM file for a game - games may consist of multiple ROM files
struct RomFile {
    const char* name;      // file name, e.g. "invaders.h"
    uint32_t    size;      // file size in bytes
    uint16_t    address;   // where the machine stores file in memory
};

struct RomLoadResult {
    std::string error;                        // "" on success; otherwise names the bad file
    std::vector<std::vector<uint8_t>> data;   // one entry per RomFile, in order (empty on error)

    bool ok() const { return error.empty(); }
};

// Reads every file in `files` from `folder` and checks each one's size.
// Returns the bytes; the machine stores them at each file's address.
RomLoadResult load_rom_files(const std::string& folder, const std::vector<RomFile>& files);

#endif
