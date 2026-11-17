#ifndef INVADERS_CONTROLS_HPP
#define INVADERS_CONTROLS_HPP

enum class Input {
    Coin,
    P1Start,
    P2Start,
    P1Left,
    P1Right,
    P1Fire,
    P2Left,
    P2Right,
    P2Fire,
    Tilt,
};

// keyboard key to arcade control mapping
struct KeyBinding {const char* key; Input input;};
const KeyBinding KEY_BINDINGS[] = {
    { "C",     Input::Coin },
    { "1",     Input::P1Start },
    { "2",     Input::P2Start },
    { "Left",  Input::P1Left },
    { "Right", Input::P1Right },
    { "Space", Input::P1Fire },
    { "A",     Input::P2Left },
    { "D",     Input::P2Right },
    { "W",     Input::P2Fire },
    { "T",     Input::Tilt },
};

#endif
