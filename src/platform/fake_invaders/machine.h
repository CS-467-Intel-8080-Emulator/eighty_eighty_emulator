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
    float get_framerate() const;
    const uint32_t* get_display_buffer() const;

    void run_frame();

private:
    char* title;
    const int display_width;
    const int display_height;
    const float framerate; // Calc based on clock speed vs. stored as variable?

    std::vector<uint32_t> pixels = std::vector<uint32_t>(display_width * display_height);

    // Temp helpers to fake animation
    uint32_t frame = 0;
    void fill_rect(int x, int y, int w, int h, uint32_t color);
};
