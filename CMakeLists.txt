cmake_minimum_required(VERSION 3.20)
project(eighty_eighty_emulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Core emulator library (src/core and src/platform -- no SDL)
# Hand-list files for now. Later, use regex to gather all core/*.cpp and platform/*.cpp
add_library(emu STATIC
    src/core/cpu.cpp
    src/core/memory.cpp
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

# The main program (currently the SDL "Fake Invaders" test)
if(WIN32)
    set(OS_NAME windows)  # 8080_windows.exe
elseif(APPLE)
    set(OS_NAME macos)    # 8080_macos
else()
    set(OS_NAME linux)    # 8080_linux
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

# Spec-based CPU suite (tests/spec): gray-box tests of every
# instruction, written from the Intel 8080 manual. Separate executable.
file(GLOB SPEC_TEST_SOURCES CONFIGURE_DEPENDS tests/spec/*.cpp)
add_executable(cpu_spec_tests ${SPEC_TEST_SOURCES})
target_include_directories(cpu_spec_tests PRIVATE tests/spec)
target_link_libraries(cpu_spec_tests PRIVATE emu Catch2::Catch2WithMain)
catch_discover_tests(cpu_spec_tests TEST_PREFIX "spec:")

# --- CPU test ROMs ---
# Download ROMs at configure time instead of committing them because two have
# no clear license. Download ROMs from a specific superzazu/8080 commit, check
# their SHA-256 values, and store them in build/test_roms/, which Git ignores.
set(ROM_COMMIT 274ffd700b81baabea99b0963bc1260b67132185)
set(ROM_URL https://raw.githubusercontent.com/superzazu/8080/${ROM_COMMIT}/cpu_tests)
set(ROMS ${CMAKE_BINARY_DIR}/test_roms)
set(ROM_HASHES
    8080PRE 18eb3c79cba42c0718f160be6a1853cb64cdce7aa47d65780189a57bdd98c4e0
    TST8080 9561c6fb6c99efe3de00eb77e4044fd102151058b39ac2d7bce10483838a08e7
    CPUTEST e61a9a75348c774486c2207080ea4effbf6c2367fdace31b0731081a4144030b
    8080EXM 6e3286e11bb1a8f47b8ee1280b4a067be813193363e3223c99b0d21912f44aeb
)
while(ROM_HASHES)
    list(POP_FRONT ROM_HASHES name hash)
    if(NOT EXISTS ${ROMS}/${name}.COM)
        file(DOWNLOAD ${ROM_URL}/${name}.COM ${ROMS}/${name}.COM
             EXPECTED_HASH SHA256=${hash} STATUS status)
        list(GET status 0 code)
        if(NOT code EQUAL 0)
            message(FATAL_ERROR "Unable to download test ROM ${name}.COM: ${status}")
        endif()
    endif()
endwhile()

# CP/M test machine that runs the ROM tests.
add_executable(cpm_test
    src/platform/cpm_test/main.cpp
    src/platform/cpm_test/cpm_machine.cpp
)
target_link_libraries(cpm_test PRIVATE emu)

# 8080PRE test
add_test(NAME 8080PRE COMMAND cpm_test ${ROMS}/8080PRE.COM)
set_tests_properties(8080PRE PROPERTIES
    PASS_REGULAR_EXPRESSION "8080 Preliminary tests complete"
    TIMEOUT 10) # Finishes instantly, 10-second timeout

# TST8080 test
add_test(NAME TST8080 COMMAND cpm_test ${ROMS}/TST8080.COM)
set_tests_properties(TST8080 PROPERTIES
    PASS_REGULAR_EXPRESSION "CPU IS OPERATIONAL"
    TIMEOUT 10) # Finishes instantly, 10-second timeout

# CPUTEST test
add_test(NAME CPUTEST COMMAND cpm_test ${ROMS}/CPUTEST.COM)
set_tests_properties(CPUTEST PROPERTIES
    PASS_REGULAR_EXPRESSION "CPU TESTS OK"
    TIMEOUT 60) # Finishes in seconds, one-minute timeout

# 8080EXM test
add_test(NAME 8080EXM COMMAND cpm_test ${ROMS}/8080EXM.COM)
set_tests_properties(8080EXM PROPERTIES
    PASS_REGULAR_EXPRESSION "Tests complete"
    TIMEOUT 600) # Finishes in minutes, ten-minute timeout

# Global test ROM properties
set_tests_properties(8080PRE TST8080 CPUTEST 8080EXM PROPERTIES
    FAIL_REGULAR_EXPRESSION "ERROR|FAIL"
    LABELS rom)