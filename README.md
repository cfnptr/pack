## Features
* Compressed file pack creation
* Runtime file pack reading

## Supported operating systems
* Ubuntu
* MacOS
* Windows

## Build requirements
* C99 compiler
* [CMake 3.10+](https://cmake.org/)

## Cloning
```
git clone https://github.com/cfnptr/pack
cd pack
git submodule update --init --recursive
```

## Building
```
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build/
cmake --build build/
```

### CMake options
| Name                 | Description                         | Default value |
| -------------------- | ----------------------------------- | ------------- |
| PACK_BUILD_UTILITIES | Build PACK utility programs         | ON            |

## Utilities

### packer
* Description: creates compressed data pack from files.
* Usage: ```packer <path-to-pack> <path-to-item>...```

### unpacker
* Description: extracts compressed data pack files.
* Usage: ```unpacker <path-to-pack>```

### pack-info
* Description: shows base packet information.
* Usage: ```pack-info <path-to-pack>```

## Third-party
* [zstd](https://github.com/facebook/zstd/) (BSD License)
