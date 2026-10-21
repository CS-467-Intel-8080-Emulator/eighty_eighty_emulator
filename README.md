# Eighty Eighty Emulator

[![CI](https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator/actions/workflows/ci.yml/badge.svg)](https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator/actions/workflows/ci.yml)

**Build Requirements**: CMake 3.20+

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
`-C Release` runs optimized tests on Windows (ignored on Linux/macOS).

Run only the Catch2 unit tests. `-LE rom` skips the [CPU Test ROMs](#cpu-test-roms) until we're ready to turn them on. CI skips them for now too.
```sh
ctest --test-dir build -C Release --output-on-failure -LE rom
```

Run individual CPU test ROMs.
```sh
ctest --test-dir build -C Release --output-on-failure -R 8080PRE # instant
ctest --test-dir build -C Release --output-on-failure -R TST8080 # instant
ctest --test-dir build -C Release --output-on-failure -R CPUTEST # takes seconds
ctest --test-dir build -C Release --output-on-failure -R 8080EXM # takes minutes
```


Run all tests: the Catch2 unit tests and the CPU test ROMs.
```sh
ctest --test-dir build -C Release --output-on-failure
```

To watch a CPU test ROM's output as it runs, run the CP/M test machine directly:
```sh
# Windows
.\build\Release\cpm_test.exe build\test_roms\TST8080.COM

# Linux/macOS
./build/cpm_test build/test_roms/TST8080.COM
```

# Run
```sh
# Windows
.\build\Release\8080_windows.exe

# macOS
./build/8080_macos

# Linux
./build/8080_linux
```

# Download and Run Generated Builds
Select a recent successful build from the [GitHub Actions](https://github.com/CS-467-Intel-8080-Emulator/eighty_eighty_emulator/actions) page, and under **Artifacts** at the bottom, download the zip file for your platform: 
   * `8080_linux.zip`
   * `8080_macos.zip`
   * `8080_windows.zip`

Then unzip and run the program.
   
**Linux**:
```sh
unzip 8080_linux.zip
./8080_linux
```

**macOS**:
```sh
unzip 8080_macos.zip
xattr -d com.apple.quarantine 8080_macos
./8080_macos
```
The `xattr -d ... line` clears the 'downloaded from the internet' flag that the browser adds.

**Windows**:

Extract the executable from the zip file and double-click `8080_windows.exe`.

# CPU Test ROMs
The CPU is checked against four standard 8080 test programs not stored in this repo. CMake downloads these tests from [superzazu/8080](https://github.com/superzazu/8080/tree/274ffd700b81baabea99b0963bc1260b67132185/cpu_tests) into `build/test_roms/` and verifies each file's checksum.

| ROM | Author | License |
|---|---|---|
| `8080PRE.COM` | Frank D. Cringle (1994), adapted for the 8080 by Ian Bartholomew (2009) | GPL v2 or later |
| `8080EXM.COM` | Frank D. Cringle and Ian Bartholomew, with pass/fail CRCs added by Mike Douglas (2013) | GPL v2 or later |
| `TST8080.COM` | Microcosm Associates (Kelly Smith), 1980; updated by Mike Douglas (2012) | None stated |
| `CPUTEST.COM` | SuperSoft Associates, 1981 (Diagnostics II suite) | None stated |
