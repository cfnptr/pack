# Build instructions

## Operating Systems

* [Window](BUILDING.md#windows)
* [macOS](BUILDING.md#macos)
* [Linux](BUILDING.md#linux)


# Windows

### 1. Install Visual Studio Community

1. Download latest version from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/downloads)
2. Launch downloaded Visual Studio installer
3. Select "Desktop development with C++" before installation
4. Then click **Install** button to begin installation

Alternatively you can install and use [CLion](https://www.jetbrains.com/clion/), [VSCode](https://code.visualstudio.com/) or any other IDE.

### 2. Install Git

* Download and install latest version from [git-cms.com](https://git-scm.com/install/windows)

You may use default Git install options or chose any other default Git editor instead of **Vim**.

### 3. Install CMake

1. Download latest release version of installer from [cmake.org](https://cmake.org/download)
2. Select "Add CMake to the system PATH for the current user" during installation


# macOS

### 1. Install Xcode IDE and tools

1. Download and install latest version from the built-in [App Store](https://apps.apple.com/app/xcode/id497799835)
2. Run ```xcode-select --install``` command using **Terminal** app to install Xcode tools

Alternatively you can install and use [CLion](https://www.jetbrains.com/clion/), [VSCode](https://code.visualstudio.com/) or any other IDE.

### 2. Install required packages

1. Install **Homebrew** package manager from [brew.sh](https://brew.sh)
2. Run ```brew update``` command using **Terminal** app to update package list
3. And run ```brew install git cmake``` command to install packages


# Linux

### 1. Install Visual Studio Code IDE

1. Download and install latest version from [code.visualstudio.com](https://code.visualstudio.com/download) or from built-in store
2. Install "[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)" and "[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)" extensions inside the **VS Code**

Alternatively you can install and use [CLion](https://www.jetbrains.com/clion/), [VSCodium](https://vscodium.com/) or any other IDE.

### 2. Install required packages

* Execute [scripts/install-packages.sh](scripts/install-packages.sh) script or

### For Ubuntu/Debian

1. Run ```sudo apt-get update``` command using **Terminal** app
2. And ```sudo apt-get install git cmake build-essential```


# Build project (Compile)

Before building the project you should clone it: ```git clone --recursive -j8 <project-url>```<br>
To build the project run one of the [scripts](scripts/) using **Terminal**, **Git Bash** or build it using **IDE**.

### Visual Studio

1. Open **Visual Studio** IDE application
2. Click "Open a project or solution" and open the project **CMakeLists.txt**
3. Wait for project CMake generation to finish
4. Click **Build -> Build All** to build the project

### Visual Studio Code (VS Code and VSCodium)

1. Open **Visual Studio Code** IDE application
3. Click **File -> Open Folder...** and open the project folder
4. Select one of the compiler **Kits** in the opened window (Clang or GCC)
5. Wait for project CMake generation to finish (may take some time)
6. Click **Build** button at the bottom bar to build the project
