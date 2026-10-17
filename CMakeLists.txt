cmake_minimum_required(VERSION 3.16)
project(eighty_eighty_emulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

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

add_executable(8080 src/main.cpp)
target_link_libraries(8080 PRIVATE SDL3::SDL3)

# --- Unit tests (Catch2) ---
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

# Tests use tests/test_memory.cpp (64K RAM) in place of src/core/memory.cpp
add_executable(cpu_tests
    tests/test_jumps.cpp
    tests/test_memory.cpp
    src/core/cpu.cpp
)
target_link_libraries(cpu_tests PRIVATE Catch2::Catch2WithMain)
