#!/bin/sh -e

cd "$(dirname "$0")"

if type cmake >/dev/null; then
    cmake -S . -B build && cmake --build build -j"$(nproc)"
else
    mkdir -p build
    g++ -Wall -Wextra -Wno-unused-parameter -std=c++17 -O3 -o build/ant ../common/*.cpp ./*.cpp
fi
