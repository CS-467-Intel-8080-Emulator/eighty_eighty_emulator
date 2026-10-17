#include "machine.h"

Machine::Machine() :
    title{ "Fake Invaders" },
    display_width{ 256 },
    display_height{ 224 },
    fps{ 60.0 }, // Calc based on clock speed vs. stored as variable?
    display_rotation{ 90.0 },
    pixels{ std::vector<uint32_t>(display_width * display_height) } {
}

char* Machine::get_title() const {
    return title;
}

int Machine::get_display_width() const {
    return display_width;
}

int Machine::get_display_height() const {
    return display_height;
}

double Machine::get_fps() const {
    return fps;
}

double Machine::get_display_rotation() const {
    return display_rotation;
}

const uint32_t* Machine::get_pixels() const {
    return pixels.data();
}

void Machine::run_frame() {
    ++frame;

    uint32_t dblue = 0xFF1D2739;
    uint32_t green = 0xFF00FF00;
    uint32_t white = 0xFFFFFFFF;

    for (int i = 0; i < pixels.size(); ++i) {
        int row = i / display_width;
        pixels[i] = i == frame % pixels.size() ? (row % 2 ? green : white) : dblue;
    }
}
