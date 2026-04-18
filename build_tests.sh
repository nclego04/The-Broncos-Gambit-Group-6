#!/bin/bash
set -e

gcc -O0 -g -std=gnu11 -Wall -Wextra -DCUNIT_TESTING -o tests/engine_test src/engine.c src/movegen.c src/algorithm.c tests/engine_test.c -lcunit
echo "Built test runner: ./tests/engine_test"

gcc -O0 -g -std=gnu11 -Wall -Wextra -DCUNIT_TESTING -o tests/algorithm_test src/engine.c src/movegen.c src/algorithm.c tests/algorithm_test.c -lcunit
echo "Built test runner: ./tests/algorithm_test"