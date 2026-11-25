# Eighty Eighty Emulator

[![CI](https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator/actions/workflows/ci.yml/badge.svg)](https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator/actions/workflows/ci.yml)

An Intel 8080 arcade emulator. It currently supports Space Invaders. There are two ways to get it:
* [Option 1: Download a release](#option-1-download-a-release). Download, add the ROM files, and run. No build tools needed.
* [Option 2: Build from source](#option-2-build-from-source). Compile, test, and run the emulator yourself.

Either way, you must supply your own Space Invaders ROM files. See [Add Game Files](#add-game-files).

**System Requirements**: We tested the emulator on the following operating systems. It may also run on other versions or distributions, but we can't guarantee it.
* Windows: Windows 10 or later
* macOS: macOS 26 or later
* Linux: Ubuntu 26.04 or later

# Option 1: Download a Release
1. On the [Releases](https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator/releases) page, under **Assets** for the newest release, download the zip file for your platform:
   * `8080_linux.zip`
   * `8080_macos.zip`
   * `8080_windows.zip`
2. Unzip the file. Keep the `assets` folder next to the executable.
3. Add the Space Invaders ROM files to `assets/roms/invaders/`. Optionally, add the sound files to `assets/sounds/invaders/`. See [Add Game Files](#add-game-files).

   The folder should look like this:
   ```
   8080             (8080.exe on Windows)
   LICENSE.md
   assets/
     fonts/
     roms/invaders/     invaders.e, invaders.f, invaders.g, invaders.h
     sounds/invaders/   0.wav ... 9.wav (optional)
   ```
4. Run the program from that folder.

   **Linux**:
   ```sh
   chmod +x 8080
   ./8080
   ```

   **macOS**:
   ```sh
   chmod +x 8080
   xattr -d com.apple.quarantine 8080
   ./8080
   ```

   **Windows**: Double-click `8080.exe`.

**Troubleshooting**:
* Windows: If a "Windows protected your PC" dialog opens when you run the executable, click **More info** and then **Run anyway**.
* If the window closes right away, run the program from a terminal in the unzipped folder to see the error message (`.\8080.exe` on Windows, `./8080` on Linux/macOS). Usually, the `assets` folder isn't next to the executable, or the ROM files are missing or misnamed.

For more ways to run the program, see [Run](#run).

# Option 2: Build from Source

## Requirements
* [CMake](https://cmake.org/download/) 3.20+
* [Git](https://git-scm.com/downloads). CMake uses it to download SDL3, SDL_ttf, and Catch2.
* A C++17 compiler:
  * Windows: [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload
  * macOS: Xcode Command Line Tools (`xcode-select --install`)
  * Linux: Installed by the dependency script in [Initial Setup](#initial-setup)

## Initial Setup
Clone the repo. Navigate into the repo and run all subsequent commands in it.
```sh
git clone https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator
cd eighty_eighty_emulator
```

Linux only: Install SDL3's build dependencies for Ubuntu/Debian. For the dependencies that other distros require, see [SDL3 Linux build dependencies](https://wiki.libsdl.org/SDL3/README-linux#build-dependencies).
```sh
sh scripts/install-linux-deps.sh
```

Add the Space Invaders ROM files to `assets/roms/invaders/`. See [Add Game Files](#add-game-files).

## Quick Start
To build, test, and run the `8080` program, run the build script for your platform.
```sh
# Windows
.\scripts\build.bat

# macOS/Linux
./scripts/build.sh
```
The script passes any positional arguments to the `8080` program. For example, `./scripts/build.sh invaders /path/to/rom/folder` runs `8080 invaders /path/to/rom/folder`. Options can go before or after the arguments. The script accepts these options:
* `-e` runs the long `8080EXM` test (skipped by default).
* `-h` prints the help.

To run individual steps, see the subsequent sections.

## Build
Create build config folder. This step is required only on the first build or any time you delete the `build` folder. `-DCMAKE_BUILD_TYPE=Release` configures optimized builds on Linux/macOS (ignored on Windows).
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

Build the project. `--config Release` configures optimized builds on Windows (ignored on Linux/macOS).
```sh
cmake --build build --config Release --parallel
```

The program is built to `build/8080` on Linux/macOS and `build\Release\8080.exe` on Windows (with the default Visual Studio generator).

## Test
Run all tests.
```sh
ctest --test-dir build --build-config Release --output-on-failure
```

Optional: Run specific tests.
```sh
# Catch2 unit tests only
ctest --test-dir build --build-config Release --output-on-failure --label-exclude cpu-test

# CPU test ROMs only
ctest --test-dir build --build-config Release --output-on-failure --label-regex cpu-test

# Individual CPU test ROMs
ctest --test-dir build --build-config Release --output-on-failure --tests-regex 8080PRE # instant
ctest --test-dir build --build-config Release --output-on-failure --tests-regex TST8080 # instant
ctest --test-dir build --build-config Release --output-on-failure --tests-regex CPUTEST # < 1s
ctest --test-dir build --build-config Release --output-on-failure --tests-regex 8080EXM # ~30s
```

# Add Game Files
The emulator looks for an `assets` folder in the folder you run it from, and then in that folder's parent. Run the program from the unzipped folder (Option 1) or from the repo root (Option 2).

**ROMs (required)**: Add the four Space Invaders ROM files to `assets/roms/invaders/`, named `invaders.e`, `invaders.f`, `invaders.g`, and `invaders.h`. You must have your own legal copy. For details, see [assets/roms/invaders/README.md](assets/roms/invaders/README.md).

**Sounds (optional)**: Add the Space Invaders WAV files to `assets/sounds/invaders/`. For the file names, see [assets/sounds/invaders/README.md](assets/sounds/invaders/README.md). Without them, the game runs silently.

# Run
The commands below show a source build, run from the repo root. For a release download, run `.\8080.exe` (Windows) or `./8080` (Linux/macOS) from the unzipped folder instead.

## Run Emulator Menu
To open the emulator menu, call the `8080` executable with no arguments. When the menu opens, select a game with the arrow keys and press **Enter** to launch it.
```sh
# Windows
.\build\Release\8080.exe

# macOS/Linux
./build/8080
```

## Run Space Invaders Directly
To bypass the menu and run Space Invaders directly, call `8080 invaders`. To specify a non-default ROM folder, call `8080 invaders [ROM_FOLDER]`:
```sh
# Windows
.\build\Release\8080.exe invaders C:\path\to\rom\folder

# macOS/Linux
./build/8080 invaders /path/to/rom/folder
```

## Run CP/M Tests Directly
To run CP/M tests, use the `8080 cpm_test TESTPROG.COM` syntax. The build downloads the CPU test ROMs to `build/test_roms/` (see [CPU Test ROMs](#cpu-test-roms)). For example:
```sh
# Windows
.\build\Release\8080.exe cpm_test build\test_roms\TST8080.COM

# macOS/Linux
./build/8080 cpm_test build/test_roms/TST8080.COM
```

## Get Help
To print the usage, use the `8080 help` syntax. Any unrecognized command also prints the usage.
```sh
# Windows
.\build\Release\8080.exe help

# macOS/Linux
./build/8080 help
```

# Controls
Menu controls:

| Key | Control |
|---|---|
| `↑` / `↓` | Select game |
| `Enter` | Launch game |
| `Esc` | Quit |

Space Invaders controls:

| Key | Control |
|---|---|
| `C` | Insert coin |
| `1` / `2` | 1-player / 2-player start |
| `←` / `→` / `Space` | Player 1 left / right / fire |
| `A` / `D` / `W` | Player 2 left / right / fire |
| `T` | Tilt |
| `Esc` | Quit |
| `F5` | Quick save state |
| `F9` | Quick load state |

To start a game, press `C` to insert a coin, then press `1` for one player or `2` for two players.

Quick save writes to `assets/states/invaders/invaders.quicksave.state`, and quick load reads from it.

# CPU Test ROMs
The CPU is checked against four standard 8080 test programs not stored in this repo. CMake downloads these tests from [superzazu/8080](https://github.com/superzazu/8080/tree/274ffd700b81baabea99b0963bc1260b67132185/cpu_tests) into `build/test_roms/` and verifies each file's checksum.

| ROM | Author | License |
|---|---|---|
| `8080PRE.COM` | Frank D. Cringle (1994), adapted for the 8080 by Ian Bartholomew (2009) | GPL v2 or later |
| `8080EXM.COM` | Frank D. Cringle and Ian Bartholomew, with pass/fail CRCs added by Mike Douglas (2013) | GPL v2 or later |
| `TST8080.COM` | Microcosm Associates (Kelly Smith), 1980; updated by Mike Douglas (2012) | None stated |
| `CPUTEST.COM` | SuperSoft Associates, 1981 (Diagnostics II suite) | None stated |

# License
This project is licensed under the MIT License. See [LICENSE.md](LICENSE.md) for details.

# Third-Party Assets
This project includes the Roboto font.

Roboto is licensed under the SIL Open Font License 1.1.
See [assets/fonts/OFL.txt](assets/fonts/OFL.txt) for details.

# Contributing
Follow the project's C++ conventions in the [Eighty Eighty Emulator C++ Style Guide](docs/STYLE.md) and review the [formatting guidelines](docs/FORMATTING.md).
