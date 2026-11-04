#pragma once

#include <array>
#include <cstdint>

class MemoryBus;

class Video {
    public:
        static constexpr uint16_t VRAM_START = 0x2400;
        static constexpr uint16_t VRAM_END = 0x3FFF;
        static constexpr int SCREEN_WIDTH = 224;
        static constexpr int SCREEN_HEIGHT = 256;

        using Framebuffer = std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT>;

        Video();

        // Convert VRAM into a framebuffer
        void update(const MemoryBus& memory);

        const Framebuffer& framebuffer() const;

    private:
        Framebuffer 
        framebuffer_;

        void clear();
        void set_pixel(int x, int y, bool enabled);
};