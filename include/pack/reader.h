// Copyright 2021-2023 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "pack/defines.h"
#include <stdbool.h>

/*
 * Pack reader structure.
 */
typedef struct PackReader_T PackReader_T;
/*
 * Pack reader instance.
 */
typedef PackReader_T* PackReader;

/*
 * Create a new file pack reader instance.
 * Returns operation Pack result.
 *
 * filePath - file path string.
 * isResourcesDirectory - read from resources directory. (macOS)
 * threadCount - concurrent access thread count.
 * packReader - pack reader instance.
 */
PackResult createFilePackReader(const char* filePath,
	bool isResourcesDirectory, uint32_t threadCount, PackReader* packReader);
/*
 * Destroys pack reader instance.
 * packReader - pack reader instance or NULL.
 */
void destroyPackReader(PackReader packReader);

/*
 * Returns pack reader item count. (MT-Safe)
 * packReader - pack reader instance.
 */
uint64_t getPackItemCount(PackReader packReader);

/*
 * Search for the pack item index. (MT-Safe)
 * Returns true if item exists.
 *
 * packReader - pack reader instance.
 * path - item path string.
 * index - item index.
 */
bool getPackItemIndex(PackReader packReader, const char* path, uint64_t* index);

/*
 * Returns pack item data size. (MT-Safe)
 *
 * packReader - pack reader instance.
 * index - item index.
 */
uint32_t getPackItemDataSize(PackReader packReader, uint64_t index);

/*
 * Returns pack item zip size, or 0 if uncompressed. (MT-Safe)
 *
 * packReader - pack reader instance.
 * index - item index.
 */
uint32_t getPackItemZipSize(PackReader packReader, uint64_t index);

/*
 * Returns pack item data offset in the file. (MT-Safe)
 *
 * packReader - pack reader instance.
 * index - item index.
 */
uint64_t getPackItemFileOffset(PackReader packReader, uint64_t index);

/*
 * Returns true if pack item is a reference to duplicate item. (MT-Safe)
 *
 * packReader - pack reader instance.
 * index - item index.
 */
bool isPackItemReference(PackReader packReader, uint64_t index);

/*
 * Returns pack item path string. (MT-Safe)
 *
 * packReader - pack reader instance.
 * index - item index.
 */
const char* getPackItemPath(PackReader packReader, uint64_t index);

/*
 * Read pack item data. (MT-Safe)
 * Return operation Pack result.
 *
 * packReader - pack reader instance.
 * itemIndex - item index.
 * buffer - item data buffer.
 * threadIndex - current thread index.
 */
PackResult readPackItemData(PackReader packReader,
	uint64_t itemIndex, uint8_t* buffer, uint32_t threadIndex);

/*
 * Returns pack ZSTD contexts. (MT-Safe)
 * packReader - pack reader instance.
 */
void** const getPackZstdContexts(PackReader packReader);

/*
 * Unpack files from the pack. (MT-Safe)
 * Returns operation Pack result.
 *
 * filePath - file path string.
 * printProgress - printf reading progress.
 */
PackResult unpackFiles(const char* filePath, bool printProgress);