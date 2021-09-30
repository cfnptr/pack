#include "pack/pack_reader.h"
#include "pack/defines.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct ItemInfo
{
	uint64_t itemSize;
	uint64_t fileOffset;
	uint64_t pathSize;
} ItemInfo;
typedef struct PackItem
{
	ItemInfo info;
	char* path;
} PackItem;

struct PackReader
{
	FILE* file;
	uint64_t itemCount;
	PackItem* items;
	PackItem searchItem;
};

inline static void destroyPackItems(
	uint64_t itemCount,
	PackItem* items)
{
	for (uint64_t i = 0; i < itemCount; i++)
	{
		free(items[i].path);
	}

	free(items);
}
inline static PackResult createPackItems(
	FILE* file,
	uint64_t itemCount,
	PackItem** _items)
{
	PackItem* items = malloc(sizeof(PackItem));

	if (items == NULL)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	for (uint64_t i = 0; i < itemCount; i++)
	{
		ItemInfo info;

		size_t readResult = fread(
			&info,
			sizeof(ItemInfo),
			1,
			file);

		if (readResult != sizeof(ItemInfo))
		{
			destroyPackItems(i, items);
			free(items);
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		char* path = malloc(
			info.pathSize * sizeof(char));

		if (path == NULL)
		{
			destroyPackItems(i, items);
			free(items);
			return FAILED_TO_ALLOCATE_PACK_RESULT;
		}

		readResult = fread(
			path,
			sizeof(char),
			info.pathSize,
			file);

		if (readResult != info.pathSize)
		{
			destroyPackItems(i, items);
			free(items);
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		PackItem* item = &items[i];
		item->info = info;
		item->path = path;
	}

	*_items = items;
	return SUCCESS_PACK_RESULT;
}

PackResult createPackReader(
	const char* filePath,
	PackReader* _packReader)
{
	assert(filePath != NULL);
	assert(_packReader != NULL);

	PackReader packReader = malloc(
		sizeof(struct PackReader));

	if (packReader == NULL)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	FILE* file = fopen(
		filePath,
		"r");

	if (file == NULL)
	{
		free(packReader);
		return FAILED_TO_OPEN_FILE_PACK_RESULT;
	}

	char header[PACK_HEADER_SIZE];

	size_t readResult = fread(
		header,
		sizeof(char),
		PACK_HEADER_SIZE,
		file);

	if (readResult != PACK_HEADER_SIZE)
	{
		fclose(file);
		free(packReader);
		return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	if (header[0] != 'P' ||
		header[1] != 'A' ||
		header[2] != 'C' ||
		header[3] != 'K')
	{
		fclose(file);
		free(packReader);
		return UNKNOWN_FILE_TYPE_PACK_RESULT;
	}

	if (header[4] != PACK_VERSION_MAJOR ||
		header[5] != PACK_VERSION_MINOR)
	{
		fclose(file);
		free(packReader);
		return BAD_FILE_VERSION_PACK_RESULT;
	}

	// Skipping PATCH version check

	if (header[7] != !PACK_LITTLE_ENDIAN)
	{
		fclose(file);
		free(packReader);
		return BAD_FILE_ENDIANNESS_PACK_RESULT;
	}

	uint64_t itemCount;

	readResult = fread(
		&itemCount,
		sizeof(uint64_t),
		1,
		file);

	if (readResult != sizeof(uint64_t))
	{
		fclose(file);
		free(packReader);
		return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	PackItem* items;

	PackResult packResult = createPackItems(
		file,
		itemCount,
		&items);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		fclose(file);
		free(packReader);
		return packResult;
	}

	packReader->file = file;
	packReader->itemCount = itemCount;
	packReader->items = items;

	memset(
		&packReader->searchItem,
		0,
		sizeof(PackItem));

	*_packReader = packReader;
	return SUCCESS_PACK_RESULT;
}
void destroyPackReader(
	PackReader packReader)
{
	if (packReader == NULL)
		return;

	destroyPackItems(
		packReader->itemCount,
		packReader->items);
	fclose(packReader->file);
	free(packReader);
}

uint64_t getPackItemCount(
	PackReader packReader)
{
	assert(packReader != NULL);
	return packReader->itemCount;
}
