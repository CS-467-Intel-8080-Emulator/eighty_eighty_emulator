#include "frontend/run_invaders.hpp"
#include "frontend/run_cpm.hpp"
#include "frontend/run_menu.hpp"

#include <SDL3/SDL_main.h> // Unused here, but must be in main() for Windows/macOS
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdio>
#include <string>


static void print_usage(const char* program) {
    std::printf("Usage:\n");
    std::printf("  %s                        Open the emulator menu and select a game\n", program);
    std::printf("  %s invaders [ROM_FOLDER]  Play Space Invaders directly from default or specified ROM folder\n", program);
    std::printf("  %s cpm_test TESTPROG.COM  Run a CP/M test program, such as a CPU test ROM\n", program);
}

int main(int argc, char* argv[]) {

    std::string command = argc > 1 ? argv[1] : "";
    int result = 1;

    // 8080
    if (argc < 2) {
        result = run_menu();
    }

    // 8080 invaders [ROM_FOLDER]
    else if (command == "invaders") {
        std::string folder = ""; // Default folder used
        if (argc >= 3) {
            folder = argv[2];
        }
        result = run_invaders(folder);
    }

    // 8080 cpm_test TESTPROG.COM
    else if (command == "cpm_test") {
        if (argc < 3) {
            std::printf("Usage: %s cpm_test TESTPROG.COM\n", argv[0]);
        } else {
            std::string test_path = argv[2];
            result = run_cpm_test(test_path);
        }
    }

    // Unknown command
    else {
        print_usage(argv[0]);
    }

    // Window cleanup
    TTF_Quit();
    SDL_Quit();
    return result;
}
