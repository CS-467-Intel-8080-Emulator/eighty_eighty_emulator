#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>

#include "run_cpm.hpp"
#include "cpm_machine.hpp"

// Runs a CP/M test program in the terminal.
int run_cpm_test(const std::string &test_path) {

    // Read CP/M test program into a buffer
    std::ifstream file(test_path, std::ios::binary); // binary mode to handle Windows line breaks
    if (!file) {
        std::printf("ERROR: Unable to open %s.\n", test_path.c_str());
        return 1;
    }
    std::vector<uint8_t> test_program((std::istreambuf_iterator<char>(file)), {});
    if (test_program.empty()) {
        std::printf("ERROR: %s is empty.\n", test_path.c_str());
        return 1;
    }

    // Create CP/M test machine and load program into it
    CpmMachine machine;
    if (!machine.load(test_program)) {
        std::printf("ERROR: %s is too big to load.\n", test_path.c_str());
        return 1;
    }

    // Run test program
    bool finished = machine.run();
    std::printf("\n");
    if (!finished) {
        std::printf("ERROR: CPU halted or exceeded max cycle count.\n");
        return 1;
    }
    std::printf("*** %llu cycles\n", (unsigned long long)machine.get_cycles());
    return 0;
}
