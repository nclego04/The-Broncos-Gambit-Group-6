#!/bin/bash
set -e

DIR="$(cd "$( dirname "$0" )" && pwd )"
"$DIR/tests/engine_test"
"$DIR/tests/evaluate_test"
"$DIR/tests/algorithm_test"
"$DIR/tests/movegen_test"