# Eighty Eighty Emulator

**Build Requirements**: CMake 3.16+

Initial setup.
```sh
git clone https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator
cd eighty_eighty_emulator
```

Create build config folder. This step is required only on the first build or any time you delete the `build` folder.
```sh
cmake -S . -B build
```

Build the project. `--config Release` is required for Windows builds. Does nothing on Linux/MacOS.
```sh
cmake --build build --config Release
```

Run tests. `-C Release` is required for Windows builds. Does nothing on Linux/MacOS. 
```sh
# Run all tests, without changing directories
(cd build && ctest -C Release)

# If using CMake 3.20+, run this instead
ctest --test-dir build -C Release
```

Run the program.
```sh
# Windows
build/Release/8080_windows.exe

# macOS
./build/8080_macos

# Linux
./build/8080_linux
```