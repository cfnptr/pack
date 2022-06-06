# PACK ![CI](https://github.com/cfnptr/pack/actions/workflows/cmake.yml/badge.svg)

A library providing runtime optimized **packing** of data into archives across different platforms.<br/>
For example can be used to read game resources.

## Features

* Compressed file pack creation
* Runtime file pack reading

## Usage example

```c
PackReader packReader;

PackResult packResult = createFilePackReader(
    "resources.pack", 0, &packReader);

if (packResult != SUCCESS_PACK_RESULT)
    abort();

const uint8_t* data;
uint32_t size;

packResult = readPackPathItemData(
    packReader, "images/sky.png", &data, &size);

if (packResult != SUCCESS_PACK_RESULT)
{
    destroyPackReader(packReader);
    abort();
}

// Process loaded data...
destroyPackReader(packReader);
```

## Supported operating systems

* Ubuntu
* MacOS
* Windows

## Build requirements

* C99 compiler
* [Git 2.30+](https://git-scm.com/)
* [CMake 3.16+](https://cmake.org/)

### CMake options

| Name                 | Description                 | Default value |
|----------------------|-----------------------------|---------------|
| PACK_BUILD_SHARED    | Build Pack shared library   | `ON`          |
| PACK_BUILD_UTILITIES | Build Pack utility programs | `ON`          |

## Cloning

```
git clone --recursive https://github.com/cfnptr/pack
```

## Utilities

### packer

* Description: creates compressed data pack from files.
* Usage: ```packer <path-to-pack> <path-to-item-1>...```

### unpacker

* Description: extracts compressed data pack files.
* Usage: ```unpacker <path-to-pack>```

### pack-info

* Description: shows pack file information.
* Usage: ```pack-info <path-to-pack>```

## Third-party

* [mpio](https://github.com/cfnptr/mpio/) (Apache-2.0 License)
* [zstd](https://github.com/facebook/zstd/) (BSD License)
