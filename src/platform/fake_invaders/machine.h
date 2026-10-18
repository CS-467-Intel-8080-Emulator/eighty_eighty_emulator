#pragma once

#include <cstdint>
#include <vector>

// Placeholder that mimics the shape of an abstract "Machine" class for the
// purpose of developing the frontend; to be replaced by actual implementation.
class Machine
{
public:
    Machine();

    char* title() const;
    int display_width() const;
    int display_height() const;
    double display_rotation() const;
    // double fps() const;
    const uint32_t* pixels() const;

    void run_frame();

private:
    char* title_;
    const int display_width_;
    const int display_height_;
    const double display_rotation_;
    // const double fps_;
    std::vector<uint32_t> pixels_;

    // Temp helper to fake animation
    uint32_t frame_ = 0;
};
