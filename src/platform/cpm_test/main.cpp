#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>

#include "cpm_machine.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::printf("Usage: cpm_test PROGRAM\n");
        return 1;
    }

    // Read CP/M program into a buffer
    std::ifstream file(argv[1], std::ios::binary); // binary mode to handle Windows line breaks
    if (!file) {
        std::printf("ERROR: Unable to open %s.\n", argv[1]);
        return 1;
    }
    std::vector<uint8_t> program((std::istreambuf_iterator<char>(file)), {});

    // Create CP/M test machine and load program into it
    CpmMachine machine;
    if (!machine.load(program)) {
        std::printf("ERROR: %s is too big to load.\n", argv[1]);
        return 1;
    }

    // Run program
    bool finished = machine.run();
    std::printf("\n");
    if (!finished) {
        std::printf("ERROR: CPU halted before the program finished.\n");
        return 1;
    }
    std::printf("*** %llu cycles\n", (unsigned long long)machine.get_cycles());
    return 0;
}
