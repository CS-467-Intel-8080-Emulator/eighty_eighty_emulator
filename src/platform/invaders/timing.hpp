#ifndef INVADERS_TIMING_HPP
#define INVADERS_TIMING_HPP

#include <cstdint>

// The NTSC television standard required a total of 262 scanlines per frame for 240p video 
// running at ~60Hz. The space invaders hardware renders 224 visible lines, then takes 38 
// lines worth of time to both blank its beam (vertical blank) and return back to the 
// top of the frame. To count 224 visible lines, the count begins at 0x20. This allows the
// count to end at 0xFF (255). Inclusive counting: 255 - 32 + 1 = 224 visible lines.
// Afterward, the remaining 38 lines are counted as the counter overflows into a 9th bit. 
// Interrupt 1 occurs at 0x80 (128), roughly mid-screen, and raises RST 1.
// Interrupt 2 is triggered at 0x100 (256), the start of vblank, and raises RST 2.

namespace invaders {
    // Clocks (from MAME https://github.com/mamedev/mame/blob/master/src/mame/midw8080/mw8080bw.h)
    inline constexpr uint32_t MASTER_CLOCK_HZ = 19'968'000;
    inline constexpr uint32_t CPU_CLOCK_HZ    = MASTER_CLOCK_HZ / 10;   // 1.9968 MHz
    inline constexpr uint32_t PIXEL_CLOCK_HZ  = MASTER_CLOCK_HZ / 4;    // 4.992 MHz

    // CRT
    inline constexpr uint32_t PIXELS_PER_LINE = 320;    // 256 visible + 64 hblank 
    inline constexpr uint32_t LINES_PER_FRAME = 262;    // 224 visible + 38 vblank
    inline constexpr uint32_t MID_SCREEN_LINE = 96;     // 1st interrupt: RST 1 raised
    inline constexpr uint32_t VBLANK_LINE     = 224;    // 2nd interrupt: RST 2 raised
    inline constexpr uint32_t VBLANK_LINES = LINES_PER_FRAME - VBLANK_LINE;

    // Cycles (128 cycles/line, 33,536 cycles/frame, 17,152 cycles to mid-screen)
    inline constexpr uint32_t CYCLES_PER_LINE  = PIXELS_PER_LINE * CPU_CLOCK_HZ / PIXEL_CLOCK_HZ;
    inline constexpr uint32_t CYCLES_PER_FRAME = CYCLES_PER_LINE * LINES_PER_FRAME;
    inline constexpr uint32_t MID_SCREEN_CYCLE = (VBLANK_LINES + MID_SCREEN_LINE) * CYCLES_PER_LINE;

    // For frame pacing (about 59.54)
    inline constexpr double FRAMES_PER_SECOND = double(CPU_CLOCK_HZ) / CYCLES_PER_FRAME;
}

#endif