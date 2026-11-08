#include "frontend/main_menu.hpp"
#include "frontend/run_invaders.hpp"
#include "platform/cpm_test/run_cpm.hpp"

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include <cstdio>
#include <string>


static void print_usage(const char* program) {
    std::printf("Usage:\n");
    std::printf("  %s                        Play Space Invaders from default ROM folder (assets/roms/invaders)\n", program);
    std::printf("  %s invaders [ROM_FOLDER]  Play Space Invaders from default or specified ROM folder\n", program);
    std::printf("  %s cpm_test TESTPROG.COM  Run a CP/M test program, such as a CPU test ROM\n", program);
}

int main(int argc, char* argv[]) {

    // 8080
    if (argc < 2) {
        int result = run_main_menu();
        return result;
    }

    std::string command = argv[1];

    // 8080 invaders [ROM_FOLDER]
    if (command == "invaders") {
        std::string folder = "assets/roms/invaders";
        if (argc >= 3) {
            folder = argv[2];
        }
        int result = run_invaders(folder);
        return result;
    }

    // 8080 cpm_test TESTPROG.COM
    if (command == "cpm_test") {
        if (argc < 3) {
            std::printf("Usage: %s cpm_test TESTPROG.COM\n", argv[0]);
            return 1;
        }
        std::string test_path = argv[2];
        int result = run_cpm_test(test_path);
        return result;
    }

    // Unknown command
    print_usage(argv[0]);
    return 1;
}
