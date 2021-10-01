#pragma once
#include "pack/defines.h"
#include <stdbool.h>

typedef struct PackReader* PackReader;

PackResult createPackReader(
	const char* filePath,
	PackReader* packReader);
void destroyPackReader(
	PackReader packReader);

uint64_t getPackItemCount(
	PackReader packReader);

bool getPackItemIndex(
	PackReader packReader,
	const char* path,
	uint64_t* index);
uint32_t getPackItemDataSize(
	PackReader packReader,
	uint64_t index);
const char* getPackItemPath(
	PackReader packReader,
	uint64_t index);

PackResult readPackItemData(
	PackReader packReader,
	uint64_t index,
	uint32_t* size,
	uint8_t** data);
void destroyPackItemData(
	PackReader packReader);

PackResult unpackItems(
	const char* packPath,
	uint64_t* itemCount,
	bool printProgress);