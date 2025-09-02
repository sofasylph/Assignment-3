#!/usr/bin/env bash
# run: ./run_tests.sh
make clean && make
: > results
: > errors
./measure
make graphs
