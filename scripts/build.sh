#!/usr/bin/env bash
set -e # Stop early if a command fails
###############################################################################
# Configure, build, test, and run 8080 program on Linux or macOS
###############################################################################

# Collect positional arguments passed to 8080 executable
args=()
while (($#)) && [[ $1 != -* ]]; do
    args+=("$1")
    shift
done

# Parse options
RUNEXM=false
HELP=false
while getopts "eh" opt; do
    case $opt in
        e) RUNEXM=true;;
        h) HELP=true;;
    esac
done

# Print help
if [ "$HELP" = true ]; then
    echo "Usage: build [8080_args] [-opts]"
    echo "Options:"
    echo "  -e: Run 8080EXM test. Skipped by default (takes ~30s to run)."
    echo "  -h: Get usage info."
    exit 0
fi

# Step 1: Configure
if [ ! -d build ]; then
    echo "Build folder does not exist. Configuring build."
    cmake -B build -DCMAKE_BUILD_TYPE=Release
else
    echo "Build folder exists. Skipping build configuration."
fi

# Step 2: Build
cmake --build build --config Release --parallel

# Step 3: Test
if [ "$RUNEXM" = true ]; then
  ctest --test-dir build --build-config Release --output-on-failure
else
  ctest --test-dir build --build-config Release --output-on-failure -E 8080EXM
fi

# Step 4: Run
echo "Starting game..."
./build/8080 "${args[@]}"
