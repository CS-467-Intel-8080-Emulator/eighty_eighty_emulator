#ifndef INVADERS_INPUT_PORTS_HPP
#define INVADERS_INPUT_PORTS_HPP

#include <cstdint>

#include "platform/machine.hpp"

// DIP switches: on/off switches on the board generally set by arcade technicians
// DIP switches are read through port 2. These defaults are factory settings
struct DipSwitches {
    uint8_t ships = 3;                  // ships per game: 3 to 6
    bool extra_ship_at_1000 = false;    // false: extra ship at 1500 points
    bool show_coin_info = true;         // show coin info on the demo screen
};

/*
Space Invaders input ports IN 0-2 using active-High where 1 = active
Port 0 is unused by the game, and bits 1-3 are tied high, so it reads 0x0E
IN 2 is unrelated to OUT 2, which sets the shift register's offset'

    Bit   Port 1           Port 2
    0     Coin             (DIP) ships, low bit  (00 = 3 ... 11 = 6)
    1     2P start         (DIP) ships, high bit
    2     1P start         Tilt (cabinet shake sensor)
    3     Always 1         (DIP) extra ship at 1000 (1) or 1500 (0) points
    4     P1 fire          P2 fire
    5     P1 left          P2 left
    6     P1 right         P2 right
    7     Not connected    (DIP) coin info on demo screen (0 = shown)
*/
class InputPorts {
public:
    void set(Input input, bool pressed);
    bool set_dips(const DipSwitches& dips);

    uint8_t read_port0() const;
    uint8_t read_port1() const;
    uint8_t read_port2() const;

private:
    // controls only. fixed bits and DIP switches are added when a port is read,
    // so releasing a control can never clear them
    uint8_t port1_ = 0;
    uint8_t port2_ = 0;
    DipSwitches dips_;
};

#endif
