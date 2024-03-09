# Pack

A [library](https://github.com/cfnptr/pack) providing **packing** of files into runtime reading optimized archives, across different platforms.<br/>
For example can be used to load game resources. (images, shaders, models, levels, etc...)

See the [documentation](https://cfnptr.github.io/pack).

## Features

* Compressed file pack creation
* Runtime optimized file pack reading
* Automatic file data deduplication
* Maximum ZSTD compression level
* Customisable compression threshold
* C and C++ implementations

## Usage example

```cpp
void packReaderExampleCPP()
{
    pack::Reader packReader("resources.pack");
    auto itemIndex = packReader.getItemIndex("textures/sky.png");
    std::vector<uint8_t> itemData;
    packReader.readItemData(itemIndex, itemData);
    // use data...
}
```

```c
void packReaderExampleC()
{
    PackReader packReader = NULL;
    PackResult packResult = createFilePackReader("resources.pack", false, 1, &packReader);
    if (packResult != SUCCESS_PACK_RESULT) abort();

    uint64_t itemIndex = 0;
    bool result = getPackItemIndex(packReader, "textures/sky.png")
    if (!result) abort();

    uint32_t dataSize = getPackItemDataSize(packReader, itemIndex);
    uint8_t* itemData = (uint8_t*)malloc(dataSize);
    if (!itemData) abort();

    packResult = readPackItemData(packReader, itemIndex, itemData, 0)
    if (packResult != SUCCESS_PACK_RESULT) { free(data); abort(); }

    // use data...
    destroyPackReader(packReader);
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

Use building [instructions](BUILDING.md) to install all required tools and libraries.

### CMake options

| Name                 | Description                 | Default value |
|----------------------|-----------------------------|---------------|
| PACK_BUILD_SHARED    | Build Pack shared library   | `ON`          |
| PACK_BUILD_UTILITIES | Build Pack utility programs | `ON`          |
| PACK_BUILD_TESTS     | Build Pack library tests    | `ON`          |

### CMake targets

| Name        | Description          | Windows | macOS    | Linux |
|-------------|----------------------|---------|----------|-------|
| pack-static | Static Pack library  | `.lib`  | `.a`     | `.a`  |
| pack-shared | Dynamic Pack library | `.dll`  | `.dylib` | `.so` |
| packer      | Packer executable    | `.exe`  |          |       |
| unpacker    | Unpacker executable  | `.exe`  |          |       |
| pack-info   | Pack info executable | `.exe`  |          |       |

## Cloning

```
git clone --recursive https://github.com/cfnptr/pack
```

## Building ![CI](https://github.com/cfnptr/pack/actions/workflows/cmake.yml/badge.svg)

* Windows: ```./scripts/build-release.bat```
* macOS / Ubuntu: ```./scripts/build-release.sh```

## Utilities

### packer

Creates compressed data pack from files.

* Usage: ```packer <pack-path> <file-path-1> <item-path-1>...```
* Example: ```packer resources.pack C:/Users/user/Desktop/sky.png images/sky.png```

### unpacker

Extracts compressed data pack files.

* Usage: ```unpacker <pack-path>```
* Example: ```unpacker resources.pack```

### pack-info

Shows pack file information.

* Usage: ```pack-info <pack-path>```
* Example: ```pack-info resources.pack```

## Third-party

* [mpio](https://github.com/cfnptr/mpio/) (Apache-2.0 License)
* [zstd](https://github.com/facebook/zstd/) (BSD License)
