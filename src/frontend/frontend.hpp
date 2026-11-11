#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <functional>
#include <utility>

#include "platform/machine.hpp"

class Frontend {
public:
    using InputHandler = std::function<void(Input input, bool pressed)>;
    Frontend(const char* title, int w_native, int h_native, double rotation, int scale);
    ~Frontend();

    Frontend(const Frontend&) = delete;
    Frontend& operator=(const Frontend&) = delete;

    void set_input_handler(InputHandler handler) { input_handler_ = std::move(handler); }
    bool handle_events();
    void present(const uint32_t* pixels);

private:
    void cleanup();

    InputHandler input_handler_;
    SDL_Window* window_{ nullptr };
    SDL_Renderer* renderer_{ nullptr };
    SDL_Texture* texture_{ nullptr };
    SDL_FRect dst_{};
    double rotation_{ 0.0 };
    int w_native_{ 0 };
};
