#!/bin/bash
set -e

timestamp=$(date +%s)

libs="-luser32 -lopengl32 -lgdi32"
warnings="-Wno-writable-strings -Wno-format-security -Wno-deprecated-declarations -Wno-switch"
includes="-Ilib -Ilib/imgui -Ilib/imgui/backends"

clang++ $includes -g src/main.cpp src/rng.cpp \
        lib/imgui/imgui.cpp \
        lib/imgui/imgui_draw.cpp \
        lib/imgui/imgui_tables.cpp \
        lib/imgui/imgui_widgets.cpp \
        lib/imgui/backends/imgui_impl_win32.cpp \
        lib/imgui/backends/imgui_impl_opengl3.cpp \
        -oFractalEngine.exe $libs $warnings