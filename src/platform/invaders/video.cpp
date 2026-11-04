#include "video.hpp"
#include "core/memory_bus.hpp"

namespace {
    constexpr int BYTES_PER_COLUMN = 32;
}

Video::Video()
{
    clear();
}

void Video::update(const MemoryBus& memory)
{
    clear();

    for (uint16_t addr = VRAM_START; addr <= VRAM_END; ++addr)
    {
        const uint8_t byte = memory.read(addr);

        const uint16_t offset = addr - VRAM_START;

        // Space Invaders stores 32 bytes per vertical column.
        const int video_x = offset / BYTES_PER_COLUMN;
        const int video_y_base = (offset % BYTES_PER_COLUMN) * 8;

        for (int bit = 0; bit < 8; ++bit)
        {
            const bool pixel = (byte & (1 << bit)) != 0;

            const int video_y = video_y_base + bit;

            // Rotated coordinates to match the original Space Invaders orientation
            const int screen_x = video_x;
            const int screen_y = SCREEN_HEIGHT - 1 - video_y;

            set_pixel(screen_x, screen_y, pixel);
        }
    }
}

const Video::Framebuffer& Video::framebuffer() const
{
    return framebuffer_;
}

void Video::clear()
{
    framebuffer_.fill(0xFF000000);
}

void Video::set_pixel(int x, int y, bool enabled)
{
    if (x < 0 || x >= SCREEN_WIDTH)
        return;

    if (y < 0 || y >= SCREEN_HEIGHT)
        return;

    framebuffer_[y * SCREEN_WIDTH + x] =
        enabled ? 0xFFFFFFFF : 0xFF000000;
}
