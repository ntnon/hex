#!/bin/bash

# Kill previous hexhex instances, ignore errors if none
pkill -f hexhex || true

# Run the game in background, redirect output if you want
./bin/Debug/hexhex &
