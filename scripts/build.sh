#!/usr/bin/env bash
set -e # Stop early if a command fails
###############################################################################
# Configure, build, test, and run 8080 program on Linux or macOS.
# This script was generated with the help of AI (duck.ai with model GPT-6 Luna).
# The accompanying transcript documents the GenAI interaction that led to this
# script:
# https://drive.google.com/file/d/1zb7rj9H_c__12t-J0Ur3wHisPcyrhMm5/view?usp=drive_link
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
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

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
