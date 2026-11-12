#include "asset_paths.hpp"

#include <stdexcept>

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
