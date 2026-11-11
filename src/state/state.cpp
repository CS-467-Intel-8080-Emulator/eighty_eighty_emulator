// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). The accompanying transcript documents the GenAI interaction that
// led to my code.
// https://drive.google.com/drive/folders/14UFLJnbF9rFUq6pEIBuArmQJAz6fYccr?usp=drive_link

#include "state/state.hpp"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <system_error>

namespace fs = std::filesystem;

void StateWriter::save_u8(uint8_t byte) {
    buffer_.push_back(byte);
}

void StateWriter::save_u16(uint16_t word) {
    save_u8(word & 0x00FF);  // Low byte
    save_u8(word >> 8);      // High byte
}

void StateWriter::save_u32(uint32_t dword) {
    save_u8(dword & 0x000000FF); // Low byte
    save_u8((dword >> 8) & 0x000000FF);
    save_u8((dword >> 16) & 0x000000FF);
    save_u8((dword >> 24) & 0x000000FF); // High byte
}

uint8_t StateReader::load_u8() {
    if (position_ >= buffer_.size())
        throw std::out_of_range("StateReader: position out of range");
    return buffer_[position_++];
}

uint16_t StateReader::load_u16() {
    uint8_t lo = load_u8();
    uint8_t hi = load_u8();
    return (uint16_t(hi) << 8) | uint16_t(lo);
}

uint32_t StateReader::load_u32() {
    uint8_t a = load_u8(); // Low byte
    uint8_t b = load_u8();
    uint8_t c = load_u8();
    uint8_t d = load_u8(); // High byte
    return (uint32_t(d) << 24) | (uint32_t(c) << 16) | (uint32_t(b) << 8) | uint32_t(a);
}

void StateFile::write(const std::string& filename, const std::vector<uint8_t>& buffer) {
    std::error_code ec;

    // Ensure directory exists; ec captures exceptions to prevent early throw
    // If create directories fails, the write will also fail and tell us why
    fs::create_directories(fs::path(filename).parent_path(), ec);

    // Attempt to write file
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    out.close();

    // Throws for either bad path or failed write; errno has the reason
    if (out.fail())
        throw std::runtime_error(
            "Can't write state file (" + std::string(std::strerror(errno)) + "): " + filename);
}

std::vector<uint8_t> StateFile::read(const std::string& filename) {
    std::error_code ec;

    // Get file size; ec captures error for missing or invalid file
    std::size_t file_size = fs::file_size(filename, ec);
    if (ec)
        throw std::runtime_error(
            "Can't read state file (" + ec.message() + "): " + filename
        );

    // Read stream into pre-allocated buffer
    std::ifstream in(filename, std::ios::binary);
    std::vector<uint8_t> buffer(file_size);
    in.read(reinterpret_cast<char*>(buffer.data()), file_size);

    if (in.fail())
        throw std::runtime_error(
            "Can't read state file (" + std::string(std::strerror(errno)) + "): " + filename
        );
    return buffer;
}
