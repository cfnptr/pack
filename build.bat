ECHO OFF

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to get CMake version, please check if it's installed.
)

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build/

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to configure CMake project.
)

cmake --build build/

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to build CMake project.
)

PAUSE