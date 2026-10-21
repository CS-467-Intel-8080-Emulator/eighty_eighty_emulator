// cpm_test: runs a CP/M CPU test ROM (e.g., TST8080.COM) and prints what it prints.
// Usage: cpm_test PROGRAM.COM
//
// ctest decides pass/fail from the output and ignores the exit code, so every
// error below prints "ERROR" (matched by FAIL_REGULAR_EXPRESSION in CMakeLists.txt).

#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>

#include "cpm_machine.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::printf("Usage: cpm_test PROGRAM.COM\n");
        return 1;
    }

    // Binary mode, so Windows doesn't translate line-ending bytes
    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::printf("ERROR: can't open %s\n", argv[1]);
        return 1;
    }
    std::vector<uint8_t> program((std::istreambuf_iterator<char>(file)), {});

    CpmMachine machine;
    if (!machine.load(program)) {
        std::printf("ERROR: %s is too big to load at 0x0100\n", argv[1]);
        return 1;
    }

    bool finished = machine.run();
    std::printf("\n");
    if (!finished) {
        std::printf("ERROR: CPU halted before the program finished\n");
        return 1;
    }

    std::printf("*** %llu cycles\n", (unsigned long long)machine.cycles());
    return 0;
}
