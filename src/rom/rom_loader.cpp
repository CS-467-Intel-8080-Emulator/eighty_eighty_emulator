#include "rom_loader.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>

namespace fs = std::filesystem;

RomLoadResult load_rom_files(const std::string& folder, const std::vector<RomFile>& files) {
    RomLoadResult result;

    for (const RomFile& file : files) {

        const std::string path = (fs::path(folder) / file.name).string();
        std::ifstream in(path, std::ios::binary); // Binary mode to handle Windows line breaks
        if (!in) {
            result.error = "Missing ROM file: " + path;
            result.data.clear();
            return result;
        }

        std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), {});
        if (bytes.size() != file.size) {
            result.error = "ROM file is wrong size (expected " + std::to_string(file.size)
                         + " bytes, got " + std::to_string(bytes.size()) + "): " + path;
            result.data.clear();
            return result;
        }

        result.data.push_back(std::move(bytes));
    }

    // TODO: verify each file's checksum
    // TODO: Accept a single combined file or a .zip
    // TODO: Accept case-insenstive files
    return result;
}
