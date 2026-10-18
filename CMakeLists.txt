cmake_minimum_required(VERSION 3.16)
project(eighty_eighty_emulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Core emulator library (src/core and src/platform -- no SDL)
# Hand-list files for now. Later, use regex to gather all core/*.cpp and platform/*.cpp
# memory.cpp is left out on purpose: tests/test_memory.cpp defines its own Memory
# functions, so linking both into unit_tests would give duplicate-symbol errors.
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
add_executable(8080
    src/main.cpp
    src/frontend/frontend.cpp
    src/platform/fake_invaders/machine.cpp
)
target_include_directories(8080 PRIVATE ${CMAKE_SOURCE_DIR})
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

# Run Catch2 unit tests
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/test_*.cpp)
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE emu Catch2::Catch2WithMain)

enable_testing()
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(unit_tests)   # registers each TEST_CASE with ctest
