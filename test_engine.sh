#!/bin/bash
set -e

DIR="$(cd "$( dirname "$0" )" && pwd )"
"$DIR/build_tests.sh"
"$DIR/run_tests.sh"