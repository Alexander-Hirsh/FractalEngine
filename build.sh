#!/bin/bash
set -e

timestamp=$(date +%s)

libs="-luser32 -lopengl32 -lgdi32"
warnings="-Wno-writable-strings -Wno-format-security -Wno-deprecated-declarations -Wno-switch"
includes="-Ilib -Ilib/Include"

clang++ $includes -g src/main.cpp src/rng.cpp -oFractalEngine.exe $libs $warnings