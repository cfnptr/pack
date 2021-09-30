#include "pack/pack_reader.h"
#include "pack/defines.h"

#include "zstd.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct ItemInfo
{
	uint64_t zipSize;
	uint64_t itemSize;
	uint64_t fileOffset;
	uint8_t pathSize;
} ItemInfo;
typedef struct PackItem
{
	ItemInfo info;
	char* path;
} PackItem;

struct PackReader
{
	ZSTD_DCtx* zstdContext;
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
		free(items[i].path);

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

	ZSTD_DCtx* zstdContext = ZSTD_createDCtx();

	if (zstdContext == NULL)
	{
		free(packReader);
		return FAILED_TO_CREATE_ZSTD_PACK_RESULT;
	}

#if __linux__ || __APPLE__
	FILE* file = fopen(
		filePath,
		"r");
#elif _WIN32
	FILE* file;

	errno_t error = fopen_s(
		file,
		filePath,
		"r");

	if (error != 0)
		file = NULL;
#else
#error Unsupported operating system
#endif

	if (file == NULL)
	{
		ZSTD_freeDCtx(zstdContext);
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
		ZSTD_freeDCtx(zstdContext);
		free(packReader);
		return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	if (header[0] != 'P' ||
		header[1] != 'A' ||
		header[2] != 'C' ||
		header[3] != 'K')
	{
		fclose(file);
		ZSTD_freeDCtx(zstdContext);
		free(packReader);
		return UNKNOWN_FILE_TYPE_PACK_RESULT;
	}

	if (header[4] != PACK_VERSION_MAJOR ||
		header[5] != PACK_VERSION_MINOR)
	{
		fclose(file);
		ZSTD_freeDCtx(zstdContext);
		free(packReader);
		return BAD_FILE_VERSION_PACK_RESULT;
	}

	// Skipping PATCH version check

	if (header[7] != !PACK_LITTLE_ENDIAN)
	{
		fclose(file);
		ZSTD_freeDCtx(zstdContext);
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
		ZSTD_freeDCtx(zstdContext);
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
		ZSTD_freeDCtx(zstdContext);
		free(packReader);
		return packResult;
	}

	packReader->zstdContext = zstdContext;
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
	ZSTD_freeDCtx(
		packReader->zstdContext);
	free(packReader);
}

uint64_t getPackItemCount(
	PackReader packReader)
{
	assert(packReader != NULL);
	return packReader->itemCount;
}

static int comparePackItems(
	const void* _a,
	const void* _b)
{
	const PackItem* a = _a;
	const PackItem* b = _b;

	int difference =
		a->info.pathSize -
		b->info.pathSize;

	if (difference != 0)
		return difference;

	return memcmp(
		a->path,
		b->path,
		a->info.pathSize);
}
bool getPackItemIndex(
	PackReader packReader,
	const char* path,
	uint64_t* index)
{
	assert(packReader != NULL);
	assert(path != NULL);
	assert(index != NULL);

	PackItem* searchItem =
		&packReader->searchItem;
	searchItem->path = (char*)path;

	PackItem* item = bsearch(
		searchItem,
		packReader->items,
		packReader->itemCount,
		sizeof(PackItem),
		comparePackItems);

	if (item == NULL)
		return false;

	*index = item - packReader->items;
	return true;
}
uint64_t getPackItemDataSize(
	PackReader packReader,
	uint64_t index)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	return packReader->items[index].info.itemSize;
}
const char* getPackItemPath(
	PackReader packReader,
	uint64_t index)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	return packReader->items[index].path;
}

PackResult readPackItemData(
	PackReader packReader,
	uint64_t index,
	void* buffer)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	assert(buffer != NULL);

	FILE* file = packReader->file;
	PackItem* item = &packReader->items[index];
	uint64_t zipSize = item->info.zipSize;

	uint8_t* zipData = malloc(
		zipSize * sizeof(uint8_t));

	if (zipData == NULL)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

#if __linux__ || __APPLE__
	int seekResult = fseeko(
		file,
		(off_t)item->info.fileOffset,
		SEEK_SET);
#elif _WIN32
	int seekResult = _fseeki64(
		file,
		(__int64)item->info.fileOffset,
		SEEK_SET);
#else
#error Unsupported operating system
#endif

	if (seekResult != 0)
	{
		free(zipData);
		return FAILED_TO_SEEK_FILE_PACK_RESULT;
	}

	size_t readResult = fread(
		zipData,
		sizeof(uint8_t),
		zipSize,
		file);

	if (readResult != zipSize)
	{
		free(zipData);
		return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	uint64_t itemSize = item->info.itemSize;

	size_t decompressResult = ZSTD_decompressDCtx(
		packReader->zstdContext,
		buffer,
		itemSize,
		zipData,
		zipSize);

	free(zipData);

	if (ZSTD_isError(decompressResult) ||
		decompressResult != itemSize)
	{
		return FAILED_TO_DECOMPRESS_PACK_RESULT;
	}

	return SUCCESS_PACK_RESULT;
}

PackResult createPackItemData(
	PackReader packReader,
	uint64_t index,
	uint64_t* _size,
	uint8_t** _data)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	assert(_size != NULL);
	assert(_data != NULL);

	uint64_t size =
		packReader->items[index].info.itemSize;
	uint8_t* data = malloc(
		size * sizeof(uint8_t));

	if (data == NULL)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	PackResult result = readPackItemData(
		packReader,
		index,
		data);

	if (result != SUCCESS_PACK_RESULT)
	{
		free(data);
		return result;
	}

	*_size = size;
	*_data = data;
	return SUCCESS_PACK_RESULT;
}
void destroyPackItemData(
	uint8_t* data)
{
	free(data);
}
