// This is a placeholder file that mimics the shape of an abstract "Machine" class
// for the purpose of developing the frontend; to be replaced by actual machine
// implementation.

#include "machine.h"

char* Machine::get_title() const {
    return title;
}

int Machine::get_display_width() const {
    return display_width;
}

int Machine::get_display_height() const {
    return display_height;
}

float Machine::get_framerate() const {
    return framerate;
}

const uint32_t* Machine::get_display_buffer() const {
    return pixels.data();
}

void Machine::run_frame() {
    ++frame;

    int width = get_display_width();
    int height = get_display_height();

    fill_rect(0, 0, width, height, 0xFF101820);                 // background
    fill_rect(0, (frame * 2) % height, width, 2, 0xFF00C8C8);   // moving line

    fill_rect(2, 2, 16, 16, 0xFFE03030);                        // top-left: red, large
    fill_rect(width - 10, 2, 8, 8, 0xFF30E030);                 // top-right: green, medium
    fill_rect(2, height - 6, 4, 4, 0xFF3060FF);                 // bottom-left: blue, small
}

// Helper function for animation
void Machine::fill_rect(int x, int y, int w, int h, uint32_t color) {
    const int x0 = std::max(x, 0), x1 = std::min(x + w, get_display_width());
    const int y0 = std::max(y, 0), y1 = std::min(y + h, get_display_height());

    for (int yy = y0; yy < y1; ++yy) {
        for (int xx = x0; xx < x1; ++xx) {
            pixels[yy * get_display_width() + xx] = color;
        }
    }
}