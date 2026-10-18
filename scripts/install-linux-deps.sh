#!/bin/sh
# SDL3 build dependencies for Ubuntu/Debian. Used by CI and the README.
# Other distros: https://wiki.libsdl.org/SDL3/README-linux#build-dependencies
set -e
sudo apt-get update
sudo apt-get install -y build-essential libx11-dev libxext-dev libxrandr-dev \
  libxcursor-dev libxi-dev libxss-dev libxtst-dev libxkbcommon-dev \
  libwayland-dev libdecor-0-dev libgl1-mesa-dev libegl1-mesa-dev \
  libasound2-dev libpulse-dev libudev-dev
