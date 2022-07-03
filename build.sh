#!/bin/bash
cd $(dirname "$BASH_SOURCE")

cmake --version > /dev/null
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to get CMake version, please check if it's installed."
    exit $status
fi

echo "Configuring project..."

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build/
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to configure CMake project."
    exit $status
fi

echo ""
echo "Building project..."

cmake --build build/ --config Release
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to build CMake project."
    exit $status
fi

exit 0
