# PACK ![CI](https://github.com/cfnptr/pack/actions/workflows/cmake.yml/badge.svg)

A library providing runtime optimized **packing** of data into archives across different platforms.

## Features

* Compressed file pack creation
* Runtime file pack reading

## Supported operating systems

* Ubuntu
* MacOS
* Windows

## Build requirements

* C99 compiler
* [Git 2.30+](https://git-scm.com/)
* [CMake 3.10+](https://cmake.org/)

## Cloning

```
git clone --recursive https://github.com/cfnptr/pack
```

### CMake options

| Name                 | Description                 | Default value |
|----------------------|-----------------------------|---------------|
| PACK_BUILD_SHARED    | Build Pack shared library   | `ON`          |
| PACK_BUILD_UTILITIES | Build Pack utility programs | `ON`          |

## Utilities

### packer

* Description: creates compressed data pack from files.
* Usage: ```packer <path-to-pack> <path-to-item>...```

### unpacker

* Description: extracts compressed data pack files.
* Usage: ```unpacker <path-to-pack>```

### pack-info

* Description: shows packet information.
* Usage: ```pack-info <path-to-pack>```

## Third-party

* [mpio](https://github.com/cfnptr/mpio/) (Apache-2.0 License)
* [zstd](https://github.com/facebook/zstd/) (BSD License)
