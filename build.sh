#!/bin/bash

cd $(dirname "$BASH_SOURCE")

if ! cmake --version ; then
    echo "Failed to get CMake version, please check if it's installed."
    exit
fi

echo ""
echo "Configuring project..."

if ! cmake -DCMAKE_BUILD_TYPE=Release -S . -B build/ ; then
    echo "Failed to configure CMake project."
    exit
fi

echo ""
echo "Building project..."

if ! cmake --build build/ --config Release ; then
    echo "Failed to build CMake project."
    exit
fi
