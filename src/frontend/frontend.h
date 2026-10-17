#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

class Frontend {
public:
    Frontend(const char* title, int w_native, int h_native, double rotation, int scale);
    ~Frontend();

    Frontend(const Frontend&) = delete;
    Frontend& operator=(const Frontend&) = delete;

    bool handle_events();
    void present(const uint32_t* pixels);

private:
    void cleanup();

    SDL_Window* window_{ nullptr };
    SDL_Renderer* renderer_{ nullptr };
    SDL_Texture* texture_{ nullptr };
    SDL_FRect dst_{};
    double rotation_{ 0.0 };
    int w_native_{ 0 };
};
