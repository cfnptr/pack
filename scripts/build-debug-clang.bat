@ECHO OFF
CD /D "%~dp0"

cmake --version > nul

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to get CMake version, please check if it's installed.
    EXIT /B %ERRORLEVEL%
)

ECHO Configuring project...

cmake -DCMAKE_BUILD_TYPE=Debug -T ClangCL -A x64 -S ../ -B ../build-debug-clang/

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to configure CMake project.
    EXIT /B %ERRORLEVEL%
)

ECHO(
ECHO Building project...

cmake --build ../build-debug-clang/ --config Debug --parallel

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to build CMake project.
    EXIT /B %ERRORLEVEL%
)

EXIT /B 0