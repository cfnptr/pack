#pragma once
#include "pack/defines.h"
#include <stdbool.h>

// Pack reader instance handle.
typedef struct PackReader* PackReader;

/*
 * Create a new pack reader instance.
 * Returns operation pack result.
 *
 * filePath - pack file path string.
 * packReader - pointer to the packReader value.
 */
PackResult createPackReader(
	const char* filePath,
	PackReader* packReader);

/*
 * Destroy pack reader instance.
 */
void destroyPackReader(
	PackReader packReader);

/*
 * Returns pack reader item count.
 */
uint64_t getPackItemCount(
	PackReader packReader);

/*
 * Search for the pack item index.
 * Returns true if item exists.
 *
 * packReader - pack reader instance.
 * path - pack item path string.
 * index - pointer to the index value.
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
 * size - pointer to the size value.
 * data - pointer to the data value.
 */
PackResult readPackItemData(
	PackReader packReader,
	uint64_t index,
	uint32_t* size,
	const uint8_t** data);

/*
 * Free pack reader buffers.
 * (Decreases reader memory usage)
 */
void freePackItemData(
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
