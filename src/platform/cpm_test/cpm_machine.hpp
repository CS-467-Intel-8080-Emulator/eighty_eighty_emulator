#ifndef CPM_MACHINE_HPP
#define CPM_MACHINE_HPP

#include <cstdint>
#include <vector>

#include "core/cpu.hpp"
#include "core/memory.hpp"

class CpmMachine {

public:
    bool load(const std::vector<uint8_t>& program);
    bool run();
    uint64_t cycles() const { return cycles_; }

private:
    // Data members
    CPU cpu;
    Memory memory;
    uint64_t cycles_ = 0;

    // Helper functions
    void bdos();
};

#endif
