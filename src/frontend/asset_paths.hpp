#ifndef ASSET_PATHS_HPP
#define ASSET_PATHS_HPP

#include <filesystem>

std::filesystem::path asset_root();
std::filesystem::path get_rom_root();

void save_rom_root(const std::filesystem::path& path);

#endif
