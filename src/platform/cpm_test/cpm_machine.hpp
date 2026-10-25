#ifndef CPM_MACHINE_HPP
#define CPM_MACHINE_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/cpu.hpp"
#include "core/memory.hpp"

/*
A simple machine running CP/M (Control Program for Microcomputers), a 1970s
operating system, used here to run 8080 test ROMs. CpmMachine models a subset
of the Basic Disk Operating System (BDOS). The primary purpose of CpmMachine
is to print what the test programs print (e.g., success/fail messages) to verify
that the CPU it's using is implemented correctly. CpmMachine uses a 64K address
space. Relevant parts of the memory map are shown:

0x0000          program ends by jumping here
  ...
0x0005          BDOS call this program implements: print output and return to caller
0x0006-0x0007   top of usable memory (0xF000)
  ...
0x0100          program loaded and started here
  ...           (program ends at 0x0100 + its size)
  ...
0xF000          top of usable memory; the stack starts here and grows down
  ...
0xFFFF          end of the 64 KB address space
*/
class CpmMachine {

public:
    bool load(const std::vector<uint8_t>& program);
    bool run();
    uint64_t get_cycles() const { return cycles; }

private:
    // Memory map addresses (see the map above)
    static constexpr uint16_t EXIT_ADDR      = 0x0000;
    static constexpr uint16_t BDOS_ADDR      = 0x0005;
    static constexpr uint16_t MEMORY_TOP_PTR = 0x0006;
    static constexpr uint16_t PROGRAM_START  = 0x0100;
    static constexpr uint16_t MEMORY_TOP     = 0xF000;
    static constexpr std::size_t MEMORY_SIZE = 0x10000;  // 64 KB address space

    // BDOS function numbers. A program requests a BDOS function by putting its
    // number in register C (plus E or DE as below), then executing CALL BDOS_ADDR.
    static constexpr uint8_t PRINT_CHAR   = 2;  // print the character in E
    static constexpr uint8_t PRINT_STRING = 9;  // print the text at DE, up to '$'

    // Data members
    CPU cpu;
    Memory memory;
    uint64_t cycles = 0;

    // Helper functions for a BDOS call (always called as a pair)
    void print_output();
    void return_to_caller();
};

#endif
