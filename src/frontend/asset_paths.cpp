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

    if (fs::exists(config_file))
    {
        std::ifstream config(config_file);

        std::string path;

        if (std::getline(config, path) &&
            !path.empty() &&
            fs::exists(path) &&
            fs::is_directory(path))
        {
            return fs::path(path);
        }
    }

    return asset_root() / "roms";
}

void save_rom_root(const fs::path& path)
{
    const fs::path config_dir =
        asset_root() / "config";

    fs::create_directories(config_dir);

    const fs::path config_file =
        config_dir / "rom_path.txt";

    std::ofstream config(config_file);

    if (!config)
    {
        throw std::runtime_error(
            "Unable to write ROM path configuration");
    }

    config << path.string();
}
