cmake_minimum_required(VERSION 3.20)
project(eighty_eighty_emulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ===============================================================
# CORE EMULATOR LIBRARY
# ===============================================================
# src/core, src/rom, and src/platform (no SDL or main.cpp).
# New .cpp files are picked up automatically on next build.
file(GLOB_RECURSE EMU_SOURCES CONFIGURE_DEPENDS
    src/core/*.cpp
    src/rom/*.cpp
    src/platform/*.cpp
)
list(FILTER EMU_SOURCES EXCLUDE REGEX "main\\.cpp$")
list(FILTER EMU_SOURCES EXCLUDE REGEX "/fake_invaders/") # TODO: Remove when platform/fake_invaders is deleted
add_library(emu STATIC ${EMU_SOURCES})
target_include_directories(emu PUBLIC src)

# ===============================================================
# SDL3
# ===============================================================
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

# ===============================================================
# SDL_TTF
# ===============================================================
# Build SDL_TTF statically so there's no SDL_TFF.dll / .so to hunt down at runtime
set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLTTF_HARFBUZZ ON CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-3.2.2
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SDL3_ttf)

# ===============================================================
# MAIN PROGRAM
# ===============================================================
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
    src/frontend/menu.cpp
    src/frontend/asset_paths.cpp
)
target_include_directories(8080 PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(8080 PRIVATE emu SDL3::SDL3 SDL3_ttf::SDL3_ttf)
set_target_properties(8080 PROPERTIES OUTPUT_NAME 8080_${OS_NAME})
install(TARGETS 8080 DESTINATION .)

# ===============================================================
# UNIT TESTS
# ===============================================================
# Catch2 and CTest setup, used by all the test sections below
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

enable_testing()
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# Unit tests (tests/test_*.cpp)
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/test_*.cpp)
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE emu Catch2::Catch2WithMain)

enable_testing()
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(unit_tests ADD_TAGS_AS_LABELS)   # registers each TEST_CASE with ctest; tags become ctest labels

# ===============================================================
# CPU SPEC TESTS
# ===============================================================
# tests/spec: gray-box tests of every instruction, written from the
# Intel 8080 manual. Separate executable.
file(GLOB SPEC_TEST_SOURCES CONFIGURE_DEPENDS tests/spec/*.cpp)
add_executable(cpu_spec_tests ${SPEC_TEST_SOURCES})
target_include_directories(cpu_spec_tests PRIVATE tests/spec)
target_link_libraries(cpu_spec_tests PRIVATE emu Catch2::Catch2WithMain)
catch_discover_tests(cpu_spec_tests TEST_PREFIX "spec:")

# ===============================================================
# CP/M TEST MACHINE AND CPU TEST ROMS
# ===============================================================
# Tests run in the order registered: unit, spec, then cpm: and cpu-test: below.

# CP/M test machine program that runs the CPU test ROMs (CpmMachine itself is in emu)
add_executable(cpm_test src/platform/cpm_test/main.cpp)
target_link_libraries(cpm_test PRIVATE emu)

# CP/M test machine unit tests (tests/cpm)
add_executable(cpm_machine_tests tests/cpm/test_cpm_machine.cpp)
target_link_libraries(cpm_machine_tests PRIVATE emu Catch2::Catch2WithMain)
catch_discover_tests(cpm_machine_tests TEST_PREFIX "cpm:")

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

# CPU test ROMs, run by the CP/M test machine. Name prefix and label defined once here.
set(CPU_TEST_PREFIX "cpu-test:")
set(CPU_TEST_LABEL  "cpu-test")
function(add_cpu_test rom pass_message timeout)
    add_test(NAME ${CPU_TEST_PREFIX}${rom} COMMAND cpm_test ${ROMS}/${rom}.COM)
    set_tests_properties(${CPU_TEST_PREFIX}${rom} PROPERTIES
        PASS_REGULAR_EXPRESSION "${pass_message}"
        FAIL_REGULAR_EXPRESSION "ERROR|FAIL"
        LABELS ${CPU_TEST_LABEL}
        TIMEOUT ${timeout})
endfunction()

add_cpu_test(8080PRE "8080 Preliminary tests complete" 10)   # finishes instantly
add_cpu_test(TST8080 "CPU IS OPERATIONAL"              10)   # finishes instantly
add_cpu_test(CPUTEST "CPU TESTS OK"                    30)   # finishes in < 1 s
add_cpu_test(8080EXM "Tests complete"                  60)   # finishes in ~30 s