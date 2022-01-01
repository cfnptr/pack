// Copyright 2021-2022 Nikita Fediuchin. All rights reserved.
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
 * Returns operation pack result.
 *
 * filePath - pack file path string.
 * packReader - pack reader instance.
 */
PackResult createFilePackReader(
	const char* filePath,
	PackReader* packReader);
/*
 * Destroys pack reader instance.
 * packReader - pack reader instance or NULL.
 */
void destroyPackReader(
	PackReader packReader);

/*
 * Returns pack reader item count.
 * packReader - pack reader instance.
 */
uint64_t getPackItemCount(
	PackReader packReader);

/*
 * Search for the pack item index.
 * Returns true if item exists.
 *
 * packReader - pack reader instance.
 * path - pack item path string.
 * index - pack item index.
 */
bool getPackItemIndex(
	PackReader packReader,
	const char* path,
	uint64_t* index);

/*
 * Returns pack item data size.
 *
 * packReader - pack reader instance.
 * index - pack reader item index.
 */
uint32_t getPackItemDataSize(
	PackReader packReader,
	uint64_t index);

/*
 * Returns pack item path string.
 *
 * packReader - pack reader instance.
 * index - pack reader item index.
 */
const char* getPackItemPath(
	PackReader packReader,
	uint64_t index);

/*
 * Read pack item data.
 * Return operation pack result.
 *
 * packReader - pack reader instance.
 * index - pack reader item index.
 * data - item data buffer.
 * size - item data size.
 */
PackResult readPackItemData(
	PackReader packReader,
	uint64_t index,
	const uint8_t** data,
	uint32_t* size);

/*
 * Read pack item data.
 * Return operation pack result.
 *
 * packReader - pack reader instance.
 * path - pack item path string.
 * data - item data buffer.
 * size - item data size.
*/
PackResult readPackPathItemData(
	PackReader packReader,
	const char* path,
	const uint8_t** data,
	uint32_t* size);

/*
 * Free pack reader buffers.
 * (Decreases reader memory usage)
 *
 * packReader - pack reader instance.
 */
void freePackReaderBuffers(
	PackReader packReader);

/*
 * Unpack files from the pack.
 * Returns operation pack result.
 *
 * filePath - pack file path string.
 * fileCount - unpacked file count.
 * printProgress - printf reading progress.
 */
PackResult unpackFiles(
	const char* filePath,
	uint64_t* fileCount,
	bool printProgress);
