// Unit tests for the ROM loader
// Using small, fake, non-copyrighted ROM files in a temporary folder

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "../src/platform/invaders/invaders_rom.hpp"
#include "../src/rom/rom_loader.hpp"

namespace fs = std::filesystem;

// A new, empty temporary folder, deleted when the test ends
struct TempDir {
    fs::path path;

    TempDir() {
        std::random_device rd;
        path = fs::temp_directory_path() / ("rom_loader_test_" + std::to_string(rd()));
        fs::create_directories(path);
    }
    ~TempDir() { fs::remove_all(path); }

    // Writes `size` bytes, all equal to `fill`, to a file in this folder
    void write(const std::string& name, std::size_t size, uint8_t fill) const {
        std::ofstream out(path / name, std::ios::binary);
        std::vector<char> bytes(size, static_cast<char>(fill));
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }
};

static const std::vector<RomFile> TWO_FILES = {
    {"a.rom", 4, 0x0000},
    {"b.rom", 2, 0x0004},
};


// --- Loading ---

TEST_CASE("Loads every file's bytes, in table order", "[rom]") {
    TempDir dir;
    dir.write("a.rom", 4, 0xAA);
    dir.write("b.rom", 2, 0xBB);

    RomLoadResult result = load_rom_files(dir.path.string(), TWO_FILES);

    REQUIRE(result.ok());
    REQUIRE(result.data.size() == 2);
    CHECK(result.data[0] == std::vector<uint8_t>{0xAA, 0xAA, 0xAA, 0xAA});
    CHECK(result.data[1] == std::vector<uint8_t>{0xBB, 0xBB});
}

TEST_CASE("Reads bytes exactly, including line-ending and Ctrl+Z bytes", "[rom]") {
    TempDir dir;
    {
        std::ofstream out(dir.path / "a.rom", std::ios::binary);
        out.write("\r\n\x1A\x00", 4);   // CR, LF, Ctrl+Z, NUL: text mode would change these
    }
    dir.write("b.rom", 2, 0x00);

    RomLoadResult result = load_rom_files(dir.path.string(), TWO_FILES);

    REQUIRE(result.ok());
    CHECK(result.data[0] == std::vector<uint8_t>{0x0D, 0x0A, 0x1A, 0x00});
}


// --- Errors ---

TEST_CASE("A missing file is reported by name", "[rom]") {
    TempDir dir;
    dir.write("a.rom", 4, 0xAA);   // b.rom is missing

    RomLoadResult result = load_rom_files(dir.path.string(), TWO_FILES);

    CHECK_FALSE(result.ok());
    CHECK(result.error.find("Missing ROM file") != std::string::npos);
    CHECK(result.error.find("b.rom") != std::string::npos);
    CHECK(result.data.empty());
}

TEST_CASE("A missing folder is reported as a missing file", "[rom]") {
    RomLoadResult result = load_rom_files("no_such_folder", TWO_FILES);

    CHECK_FALSE(result.ok());
    CHECK(result.error.find("a.rom") != std::string::npos);
}

TEST_CASE("A file of the wrong size is reported with both sizes", "[rom]") {
    TempDir dir;
    dir.write("a.rom", 3, 0xAA);   // should be 4
    dir.write("b.rom", 2, 0xBB);

    RomLoadResult result = load_rom_files(dir.path.string(), TWO_FILES);

    CHECK_FALSE(result.ok());
    CHECK(result.error.find("wrong size") != std::string::npos);
    CHECK(result.error.find("expected 4") != std::string::npos);
    CHECK(result.error.find("got 3") != std::string::npos);
    CHECK(result.data.empty());
}


// --- The Space Invaders table ---

TEST_CASE("Space Invaders' ROM files fill 0x0000-0x1FFF with no gaps", "[rom][invaders]") {
    uint32_t next_address = 0x0000;
    for (const RomFile& file : INVADERS_ROM_FILES) {
        CHECK(file.address == next_address);
        CHECK(file.size == 0x800);
        next_address = file.address + file.size;
    }
    CHECK(next_address == 0x2000);
}

TEST_CASE("Space Invaders' ROM loads from four fake 2 KB files", "[rom][invaders]") {
    TempDir dir;
    for (const RomFile& file : INVADERS_ROM_FILES) {
        dir.write(file.name, file.size, 0x00);
    }

    RomLoadResult result = load_rom_files(dir.path.string(), INVADERS_ROM_FILES);

    REQUIRE(result.ok());
    CHECK(result.data.size() == 4);
}
