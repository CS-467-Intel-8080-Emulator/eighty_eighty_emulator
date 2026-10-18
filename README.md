# Eighty Eighty Emulator

[![CI](https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator/actions/workflows/ci.yml/badge.svg)](https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator/actions/workflows/ci.yml)

**Build Requirements**: CMake 3.16+

# Initial Setup
Clone the repo. Navigate into the repo and run all subsequent commands in it.
```sh
git clone https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator
cd eighty_eighty_emulator
```

Linux only: Install SDL3's build dependencies for Ubuntu/Debian. For the dependencies that other distros require, see [SDL3 Linux build dependencies](https://wiki.libsdl.org/SDL3/README-linux#build-dependencies).
```sh
sh scripts/install-linux-deps.sh
```

# Build
Create build config folder. This step is required only on the first build or any time you delete the `build` folder. `-DCMAKE_BUILD_TYPE=Release` configures optimized builds on Linux/macOS (ignored on Windows).
```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

Build the project. `--config Release` configures optimized builds on Windows (ignored on Linux/macOS).
```sh
cmake --build build --config Release --parallel
```

# Test
`--C Release` run optimized tests on Windows (ignored on Linux/macOS).
```sh
# Run all tests, without changing directories
(cd build && ctest -C Release --output-on-failure)

# If using CMake 3.20+, run this instead
ctest --test-dir build -C Release --output-on-failure
```

## Run
```sh
# Windows
.\build\Release\8080_windows.exe

# macOS
./build/8080_macos

# Linux
./build/8080_linux
```