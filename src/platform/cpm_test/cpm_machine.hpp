#ifndef CPM_MACHINE_HPP
#define CPM_MACHINE_HPP

#include <cstdint>
#include <vector>

#include "core/cpu.hpp"
#include "core/memory.hpp"

/*
Simple CP/M (Control Program/Monitor) machine for running the 8080 test ROMs
(TST8080.COM, CPUTEST.COM, etc.). Contains 64 KB of RAM, with the test ROM
(a CP/M program) loaded and started at 0x0100. The ROM prints its own output
via the machine's BDOS (Basic Disk Operating System) call stored at 0x0005,
and finishes by jumping to 0x0000.
*/
class CpmMachine {
public:
    bool load(const std::vector<uint8_t>& program);
    bool run();
    uint64_t cycles() const { return cycles_; }

private:
    void bdos();

    CPU cpu;
    Memory memory; // its functions are defined in cpm_machine.cpp
    uint64_t cycles_ = 0;
};

#endif
