#include "rom_reader.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace fs = std::filesystem;

std::vector<RomFile> read_rom_files(const std::string& folder, const std::vector<RomFile>& files) {

    // Check that folder exists
    if (!fs::is_directory(folder))
        throw std::runtime_error("ROM folder not found: " + folder);

    std::vector<RomFile> rom_files_read;

    // TODO: Accept a single combined file or a .zip
    // TODO: Accept case-insensitive files

    for (const RomFile& file : files) {

        // Check that file exists
        const std::string path = (fs::path(folder) / file.name).string();
        std::ifstream in(path, std::ios::binary); // Binary mode to handle Windows line breaks
        if (!in)
            throw std::runtime_error("Missing ROM file: " + path);

        // Read file's bytes and check its size
        std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), {});
        if (bytes.size() != file.size)
            throw std::runtime_error("ROM file is wrong size (expected " + std::to_string(file.size)
                                     + " bytes, got " + std::to_string(bytes.size()) + "): " + path);

        // TODO: verify each file's checksum

        RomFile rom_file_read = file;
        rom_file_read.bytes = std::move(bytes);
        rom_files_read.push_back(std::move(rom_file_read));
    }

    return rom_files_read;
}
