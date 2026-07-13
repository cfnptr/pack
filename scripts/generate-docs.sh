#!/bin/bash
cd "$(dirname "$BASH_SOURCE")"

if ! doxygen --version &> /dev/null; then
    echo "Failed to get Doxygen version, please check if it's installed."
    exit 1
fi
if ! git --version &> /dev/null; then
    echo "Failed to get Git version, please check if it's installed."
    exit 1
fi

if [ ! -d "../doxygen/cppreference" ]; then
    git clone https://github.com/cfnptr/cppreference-doxygen ../doxygen/cppreference
else
    git -C ../doxygen/cppreference pull
fi
status=$?
if [ $status -ne 0 ]; then
    echo "Failed to clone/pull cppreference-doxygen repository."
    exit $status
fi

if [ ! -d "../doxygen/awesome-css" ]; then
    git clone https://github.com/jothepro/doxygen-awesome-css ../doxygen/awesome-css
else
    git -C ../doxygen/awesome-css pull
fi
status=$?
if [ $status -ne 0 ]; then
    echo "Failed to clone/pull doxygen-awesome-css repository."
    exit $status
fi

cd "../"

doxygen Doxyfile
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to generate documentation."
    exit $status
fi

exit 0
