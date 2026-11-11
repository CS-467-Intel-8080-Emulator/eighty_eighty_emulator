// This code was generated with the help of Claude (Claude Code, model Claude
// Opus 5.5). The accompanying transcript documents the GenAI interaction that
// led to my code.
// https://drive.google.com/drive/folders/14UFLJnbF9rFUq6pEIBuArmQJAz6fYccr?usp=drive_link

#ifndef STATE_HPP
#define STATE_HPP

#include <vector>
#include <cstdint>
#include <string>

// Serializes machine state to a byte buffer.
class StateWriter {
public:
    void save_u8(uint8_t value);
    void save_u16(uint16_t value);
    void save_u32(uint32_t value);

    const std::vector<uint8_t>& get_buffer() const { return buffer_; }

private:
    std::vector<uint8_t> buffer_;

};

// Deserializes machine state from a byte buffer. Machine component state loads
// must be performed in the same order as they were written to the buffer.
class StateReader {
public:
    uint8_t load_u8();
    uint16_t load_u16();
    uint32_t load_u32();

    void set_buffer(const std::vector<uint8_t>& buffer) {
        buffer_ = buffer;
        position_ = 0;
    }

private:
    std::vector<uint8_t> buffer_;
    std::size_t position_ = 0;
};

// Utility class for writing/reading state buffer to/from a file.
// Both functions throw std::runtime_error with the reason and path on failure.
class StateFile {
public:
    static void write(const std::string& filename, const std::vector<uint8_t>& buffer);
    static std::vector<uint8_t> read(const std::string& filename);
};

#endif
