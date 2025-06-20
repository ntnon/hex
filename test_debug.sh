#!/bin/bash

# Test script to run the game and capture debug output

echo "Building the game..."
make

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Running game to capture debug output..."

# Run the game in background and capture output
./bin/Debug/raylib-quickstart > debug_output.txt 2>&1 &
GAME_PID=$!

# Let it run for longer to capture more output
sleep 8

# Kill the game
kill $GAME_PID 2>/dev/null

# Wait a moment for the process to clean up
sleep 1

echo "Debug output:"
echo "=============="
cat debug_output.txt | head -100

echo ""
echo "Looking for edge-related output:"
echo "================================"
grep -i "edge\|created\|drawn\|initializing\|hex board" debug_output.txt || echo "No edge-related output found"

echo ""
echo "Full debug output (last 30 lines):"
echo "===================================="
tail -30 debug_output.txt

# Clean up
rm -f debug_output.txt

echo ""
echo "Test complete."
