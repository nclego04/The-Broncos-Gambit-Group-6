@echo off
gcc -O2 -std=gnu11 -Wall -Wextra -o engine.exe src/engine.c src/movegen.c src/algorithm.c
echo Built engine.exe