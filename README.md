# Pack

A library providing **packing** of files into runtime reading optimized archives, across different platforms.<br/>
For example can be used to load game resources. (images, shaders, models, levels, etc...)

See the [documentation](docs/html/index.html).

## Features

* Compressed file pack creation
* Runtime optimized file pack reading
* Automatic file data deduplication
* Maximum ZSTD compression level
* Customisable compression threshold
* C and C++ implementations

## Usage example

```cpp
void packReaderExample()
{
   pack::Reader packReader("resources.pack");
   auto itemIndex = packReader.getItemIndex("textures/sky.png");
   auto dataSize = packReader.getItemDataSize(itemIndex);
   std::vector<uint8_t> itemData(dataSize);
   packReader.readItemData(itemIndex, itemData.data());
}
```

## Supported operating systems

* Windows
* macOS
* Ubuntu (Linux)

## Build requirements

* C99 compiler
* C++17 compiler (optional)
* [Git 2.30+](https://git-scm.com/)
* [CMake 3.16+](https://cmake.org/)

### CMake options

| Name                 | Description                 | Default value |
|----------------------|-----------------------------|---------------|
| PACK_BUILD_SHARED    | Build Pack shared library   | `ON`          |
| PACK_BUILD_UTILITIES | Build Pack utility programs | `ON`          |
| PACK_BUILD_TESTS     | Build Pack library tests    | `ON`          |

## Cloning

```
git clone --recursive https://github.com/cfnptr/pack
```

## Building ![CI](https://github.com/cfnptr/pack/actions/workflows/cmake.yml/badge.svg)

* Windows: ```./scripts/build-release.bat```
* macOS / Ubuntu: ```./scripts/build-release.sh```

## Utilities

### packer

* Description: creates compressed data pack from files.
* Usage: ```packer <pack-path> <file-path-1> <item-path-1>...```

### unpacker

* Description: extracts compressed data pack files.
* Usage: ```unpacker <pack-path>```

### pack-info

* Description: shows pack file information.
* Usage: ```pack-info <pack-path>```

## Third-party

* [mpio](https://github.com/cfnptr/mpio/) (Apache-2.0 License)
* [zstd](https://github.com/facebook/zstd/) (BSD License)