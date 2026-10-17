cmake_minimum_required(VERSION 3.16)
project(eighty_eighty_emulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Core emulator library (src/core and src/platform -- no SDL)
# Hand-list files for now. Later, use regex to gather all core/*.cpp and platform/*.cpp
# memory.cpp is left out on purpose: tests/test_memory.cpp defines its own Memory
# functions, so linking both into test_alu would give duplicate-symbol errors.
add_library(emu STATIC
    src/core/cpu.cpp
)
target_include_directories(emu PUBLIC src)

# Build SDL statically so there's no SDL3.dll / .so to hunt down at runtime
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.18
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL3)

# The main program (currently the SDL "Hello, world!" test - to be replaced later by src/frontend)
# The file is named for the OS: 8080_windows.exe, 8080_macos, or 8080_linux.
if(WIN32)
    set(OS_NAME windows)
elseif(APPLE)
    set(OS_NAME macos)
else()
    set(OS_NAME linux)
endif()
add_executable(8080 src/main.cpp)
target_link_libraries(8080 PRIVATE SDL3::SDL3)
set_target_properties(8080 PROPERTIES OUTPUT_NAME 8080_${OS_NAME})
install(TARGETS 8080 DESTINATION .)

# --- Unit tests (Catch2) ---
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

# Planned: one Catch2 program for every tests/test_*.cpp file, so new test files
# don't need to be added here (just re-run `cmake -B build`).
# Name test files tests/test_<component>.cpp. Other .cpp files in tests/ are left out,
# so put shared helpers in headers (like cpu_test_helpers.hpp).
# Switch to this once test_alu.cpp is converted to Catch2 (it has its own main() for now),
# then delete the cpu_tests and test_alu targets and their corresponding add_test lines.
# Updated code would look something like this:
#
# file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/test_*.cpp)
# add_executable(unit_tests ${TEST_SOURCES})
# target_link_libraries(unit_tests PRIVATE emu Catch2::Catch2WithMain)
#
# list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
# include(Catch)
# catch_discover_tests(unit_tests)   # registers each TEST_CASE with ctest

# Tests use tests/test_memory.cpp (64K RAM) in place of src/core/memory.cpp
add_executable(cpu_tests
    tests/test_jumps.cpp
    tests/test_memory.cpp
    src/core/cpu.cpp
)
target_link_libraries(cpu_tests PRIVATE Catch2::Catch2WithMain)

# Unit tests. In the future, consider replacing with Catch2
add_executable(test_alu
    tests/test_alu.cpp
    tests/test_memory.cpp
)
target_link_libraries(test_alu PRIVATE emu Catch2::Catch2WithMain)

enable_testing()
add_test(NAME test_alu COMMAND test_alu)
add_test(NAME cpu_tests COMMAND cpu_tests)
