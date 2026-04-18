#!/bin/bash
set -e
gcc -O2 -std=gnu11 -Wall -Wextra -o engine src/main.c src/engine.c src/movegen.c src/algorithm.c src/evaluate.c
echo "Built ./engine"
