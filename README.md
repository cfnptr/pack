# PACK ![CI](https://github.com/cfnptr/pack/actions/workflows/cmake.yml/badge.svg)

A library providing **packing** of files into runtime reading optimized archives, across different platforms.<br/>
For example can be used to read game resources. (images, shaders, models, levels, etc...)

## Features

* Compressed file pack creation
* Runtime optimized file pack reading
* Duplicate file data detection
* Maximum ZSTD compression level
* C++ library wrapper

## Usage example

```c++
void packReaderExample()
{
   pack::Reader packReader("resources.pack");
   auto itemIndex = packReader.getItemIndex("textures/sky.png");
   auto dataSize = packReader.getItemDataSize(itemIndex);
   vector<uint8_t> itemData(dataSize);
   packReader.readItemData(itemIndex, itemData.data());
}
```

## Supported operating systems

* Ubuntu
* MacOS
* Windows

## Build requirements

* C99 compiler
* C++11 compiler (optional)
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