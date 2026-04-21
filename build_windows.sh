#!/bin/bash
set -e

x86_64-w64-mingw32-gcc -O2 -std=gnu11 -Wall -Wextra -o engine.exe src/main.c src/engine.c src/movegen.c src/algorithm.c src/evaluate.c
echo "Cross-compiled ./engine.exe for Windows"
