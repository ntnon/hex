#!/bin/sh
set -e

echo "Running all tests..."

for test in ./tile_map_test ./other_test ./another_test; do
    if [ -x "$test" ]; then
        echo "Running $test"
        "$test"
    else
        echo "Test $test not found or not executable"
    fi
done

echo "All tests completed."


