#include "asset_paths.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

fs::path asset_root()
{
    if (fs::exists("assets"))
    {
        return "assets";
    }

    if (fs::exists("../assets"))
    {
        return "../assets";
    }

    throw std::runtime_error(
        "Unable to locate assets directory");
}

fs::path get_rom_root()
{
    const fs::path config_file =
        asset_root() /
        "config" /
        "rom_path.txt";

    if (!fs::exists(config_file))
    {
        return asset_root() / "roms";
    }

    std::ifstream config(config_file);

    std::string path_string;

    if (!std::getline(config, path_string) ||
        path_string.empty())
    {
        return asset_root() / "roms";
    }

    fs::path rom_path(path_string);

    // Resolve relative paths against the project root.
    if (rom_path.is_relative())
    {
        rom_path =
            asset_root().parent_path() /
            rom_path;
    }

    rom_path = fs::weakly_canonical(rom_path);

    if (!fs::exists(rom_path) ||
        !fs::is_directory(rom_path))
    {
        throw std::runtime_error(
            "Configured ROM directory does not exist: "
            + rom_path.string());
    }

    return rom_path;
}
