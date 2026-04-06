#!/bin/bash
cd "$(dirname "$BASH_SOURCE")"

if command -v apt-get &> /dev/null; then
    sudo apt-get update && sudo apt-get install -y git cmake build-essential
elif command -v dnf &> /dev/null; then
    sudo dnf check-update && sudo dnf install -y git cmake @c-development
elif command -v pacman &> /dev/null; then
    sudo pacman -Syu --noconfirm git cmake base-devel
elif command -v zypper &> /dev/null; then
    sudo zypper install -y git cmake -t pattern devel_basis
elif command -v apk &> /dev/null; then
    apk add --no-cache git cmake build-base
elif command -v brew &> /dev/null; then
    brew update && brew install git cmake
else
    echo "No supported package manager found."
    exit 1
fi
