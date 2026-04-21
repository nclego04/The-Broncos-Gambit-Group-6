#!/bin/bash
set -e

gcc -O0 -g -std=gnu11 -Wall -Wextra -DCUNIT_TESTING -o tests/engine_test src/engine.c src/movegen.c src/algorithm.c tests/engine_test.c src/evaluate.c -lcunit
echo "Built test runner: ./tests/engine_test"

gcc -O0 -g -std=gnu11 -Wall -Wextra -DCUNIT_TESTING -o tests/evaluate_test src/engine.c src/movegen.c src/algorithm.c tests/evaluate_test.c src/evaluate.c -lcunit
echo "Built test runner: ./tests/evaluate_test"