#!/bin/bash

# Simple build and run script for Ring Topology Demo

echo "Building Ring Topology Demo..."

# Clean previous build
make clean

# Build the project
make

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Running simulation..."
    ./demo
else
    echo "Build failed!"
    exit 1
fi