// Unit tests for the ROM reader
// Using small, fake, non-copyrighted ROM files in a temporary folder

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "../src/platform/invaders/invaders_rom.hpp"
#include "../src/rom/rom_reader.hpp"

namespace fs = std::filesystem;
using Catch::Matchers::ContainsSubstring;

// A new, empty temporary folder, deleted when the test ends
struct TempDir {
    fs::path path;

    TempDir() {
        std::random_device rd;
        path = fs::temp_directory_path() / ("rom_reader_test_" + std::to_string(rd()));
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


// --- Reading ---

TEST_CASE("Reads every file's bytes with its name and address, in table order", "[rom]") {
    TempDir dir;
    dir.write("a.rom", 4, 0xAA);
    dir.write("b.rom", 2, 0xBB);

    std::vector<RomFile> rom_files = read_rom_files(dir.path.string(), TWO_FILES);

    REQUIRE(rom_files.size() == 2);
    CHECK(std::string(rom_files[0].name) == "a.rom");
    CHECK(rom_files[0].address == 0x0000);
    CHECK(rom_files[0].bytes == std::vector<uint8_t>{0xAA, 0xAA, 0xAA, 0xAA});
    CHECK(std::string(rom_files[1].name) == "b.rom");
    CHECK(rom_files[1].address == 0x0004);
    CHECK(rom_files[1].bytes == std::vector<uint8_t>{0xBB, 0xBB});
}

TEST_CASE("Reads bytes exactly, including line-ending and Ctrl+Z bytes", "[rom]") {
    TempDir dir;
    {
        std::ofstream out(dir.path / "a.rom", std::ios::binary);
        out.write("\r\n\x1A\x00", 4);   // CR, LF, Ctrl+Z, NUL: text mode would change these
    }
    dir.write("b.rom", 2, 0x00);

    std::vector<RomFile> rom_files = read_rom_files(dir.path.string(), TWO_FILES);

    REQUIRE(rom_files.size() == 2);
    CHECK(rom_files[0].bytes == std::vector<uint8_t>{0x0D, 0x0A, 0x1A, 0x00});
}


// --- Errors ---

TEST_CASE("A missing file is reported by name", "[rom]") {
    TempDir dir;
    dir.write("a.rom", 4, 0xAA);   // b.rom is missing

    CHECK_THROWS_WITH(read_rom_files(dir.path.string(), TWO_FILES),
                      ContainsSubstring("Missing ROM file") && ContainsSubstring("b.rom"));
}

TEST_CASE("A missing folder is reported by name", "[rom]") {
    CHECK_THROWS_WITH(read_rom_files("no_such_folder", TWO_FILES),
                      ContainsSubstring("ROM folder not found") && ContainsSubstring("no_such_folder"));
}

TEST_CASE("A file of the wrong size is reported with both sizes", "[rom]") {
    TempDir dir;
    dir.write("a.rom", 3, 0xAA);   // should be 4
    dir.write("b.rom", 2, 0xBB);

    CHECK_THROWS_WITH(read_rom_files(dir.path.string(), TWO_FILES),
                      ContainsSubstring("wrong size") && ContainsSubstring("expected 4")
                      && ContainsSubstring("got 3"));
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

TEST_CASE("Reads Space Invaders' four fake 2 KB ROM files", "[rom][invaders]") {
    TempDir dir;
    for (const RomFile& file : INVADERS_ROM_FILES) {
        dir.write(file.name, file.size, 0x00);
    }

    std::vector<RomFile> rom_files = read_rom_files(dir.path.string(), INVADERS_ROM_FILES);

    CHECK(rom_files.size() == 4);
}
