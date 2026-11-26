# ROM Path Configuration

By default, the emulator searches for ROMs in:

assets/roms

To use a different ROM location, enter a path in:

assets/config/rom_path.txt

If the file is empty, the default ROM directory will be used.

Both absolute and relative paths are supported.

## Expected Directory Structure

The configured ROM directory must contain one subdirectory per game.

Example:

ROM_Holder/
├── invaders/
│   ├── invaders.h
│   ├── invaders.g
│   ├── invaders.f
│   └── invaders.e
├── lunar_rescue/
│   └── ...
└── space_chaser/
    └── ...

ROM files placed directly in the configured ROM directory will not be discovered by the menu.