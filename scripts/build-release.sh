#!/bin/bash
cd "$(dirname "$BASH_SOURCE")"

if ! cmake --version &> /dev/null; then
    echo "Failed to get CMake version, please check if it's installed."
    exit 1
fi

echo "Configuring project..."

cmake -DCMAKE_BUILD_TYPE=Release -S ../ -B ../build-release/
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to configure CMake project."
    exit $status
fi

echo ""
echo "Building project..."

cmake --build ../build-release/ --config Release --parallel
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to build CMake project."
    exit $status
fi

exit 0
