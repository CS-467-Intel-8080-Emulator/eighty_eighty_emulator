cmake_minimum_required(VERSION 3.16)
project(eighty_eighty_emulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.18
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL3)

add_executable(frontend
    src/frontend/main.cpp
    src/platform/fake_invaders/machine.cpp
)

target_include_directories(frontend
    PRIVATE
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(frontend PRIVATE SDL3::SDL3)
