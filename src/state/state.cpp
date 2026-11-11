#include "state/state.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>

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
        throw std::out_of_range(
            "StateReader: position out of range"
        );
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

void StateFile::save(const std::string& filename, const std::vector<uint8_t>& buffer) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cout << "Unable to open state file for writing: " << filename << std::endl;
        return;
    }
    out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

std::vector<uint8_t> StateFile::load(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cout << "Unable to open state file for reading: " << filename << std::endl;
        return {};
    }
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    return buffer;
}
