// This is a placeholder file that mimics the shape of an abstract "Machine" class
// for the purpose of developing the frontend; to be replaced by actual machine
// implementation.

#pragma once

#include <cstdint>
#include <vector>

// Need to abstract as a shared interface between different machines so frontend
// knows how to interact with different games
class Machine
{
public:
    char* get_title() const;
    int get_display_width() const;
    int get_display_height() const;
    float get_framerate() const;
    const uint32_t* get_display_buffer() const;

    void run_frame();

private:
    char* title = "Fake Invaders";
    const int display_width = 224;
    const int display_height = 256;
    const float framerate = 60.0;  // Will this be calculated or stored as constants?

    std::vector<uint32_t> pixels = std::vector<uint32_t>(display_width * display_height);
    uint32_t frame = 0;

    // Helper function to fake animation
    void fill_rect(int x, int y, int w, int h, uint32_t color);
};
