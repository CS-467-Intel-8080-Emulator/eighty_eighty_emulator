#include "machine.h"

Machine::Machine() :
    title_{ "Fake Invaders" },
    display_width_{ 256 },
    display_height_{ 224 },
    display_rotation_{ 90.0 },
    // fps_ ? { 60.0 }, // Calc based on clock speed vs. stored as variable?
    pixels_{ std::vector<uint32_t>(display_width_ * display_height_) } {
}

char* Machine::title() const {
    return title_;
}

int Machine::display_width() const {
    return display_width_;
}

int Machine::display_height() const {
    return display_height_;
}

// double Machine::fps() const {
//     return fps_;
// }

double Machine::display_rotation() const {
    return display_rotation_;
}

const uint32_t* Machine::pixels() const {
    return pixels_.data();
}

void Machine::run_frame() {
    ++frame_;

    uint32_t dblue = 0xFF1D2739;
    uint32_t green = 0xFF00FF00;
    uint32_t white = 0xFFFFFFFF;

    for (int i = 0; i < pixels_.size(); ++i) {
        int row = i / display_width_;
        pixels_[i] = i == frame_ % pixels_.size() ? (row % 2 ? green : white) : dblue;
    }
}
