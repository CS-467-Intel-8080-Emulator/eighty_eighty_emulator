#pragma once

#include <cstdint>
#include <vector>

// Placeholder that mimics the shape of an abstract "Machine" class for the
// purpose of developing the frontend; to be replaced by actual implementation.
class Machine
{
public:
    Machine();

    char* get_title() const;
    int get_display_width() const;
    int get_display_height() const;
    double get_fps() const;
    double get_display_rotation() const;
    const uint32_t* get_pixels() const;

    void run_frame();

private:
    char* title;
    const int display_width;
    const int display_height;
    const double fps;
    const double display_rotation;
    std::vector<uint32_t> pixels;

    // Temp helper to fake animation
    uint32_t frame = 0;
};
