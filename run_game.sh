#!/bin/bash

# Kill previous raylib-quickstart instances, ignore errors if none
pkill -f raylib-quickstart || true

# Run the game in background, redirect output if you want
./bin/Debug/raylib-quickstart &
