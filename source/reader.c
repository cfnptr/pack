#include "pack/reader.h"
#include "pack/file.h"

#include "zstd.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct PackItem
{
	PackItemInfo info;
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
	FILE* packFile,
	uint64_t itemCount,
	PackItem** _items)
{
	PackItem* items = malloc(
		itemCount * sizeof(PackItem));

	if (items == NULL)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	for (uint64_t i = 0; i < itemCount; i++)
	{
		PackItemInfo info;

		size_t result = fread(
			&info,
			sizeof(PackItemInfo),
			1,
			packFile);

		if (result != 1)
		{
			destroyPackItems(i, items);
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		if (info.itemSize == 0 ||
			info.pathSize == 0)
		{
			destroyPackItems(i, items);
			return BAD_DATA_SIZE_PACK_RESULT;
		}

		char* path = malloc(
			(info.pathSize + 1) * sizeof(char));

		if (path == NULL)
		{
			destroyPackItems(i, items);
			return FAILED_TO_ALLOCATE_PACK_RESULT;
		}

		result = fread(
			path,
			sizeof(char),
			info.pathSize,
			packFile);

		path[info.pathSize] = 0;

		if (result != info.pathSize)
		{
			destroyPackItems(i, items);
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		int64_t fileOffset = info.zipSize > 0 ?
			info.zipSize : info.itemSize;

		int seekResult = seekFile(
			packFile,
			fileOffset,
			SEEK_CUR);

		if (seekResult != 0)
		{
			destroyPackItems(i, items);
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
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

	FILE* file = openFile(
		filePath,
		"rb");

	if (file == NULL)
	{
		ZSTD_freeDCtx(zstdContext);
		free(packReader);
		return FAILED_TO_OPEN_FILE_PACK_RESULT;
	}

	char header[PACK_HEADER_SIZE];

	size_t result = fread(
		header,
		sizeof(char),
		PACK_HEADER_SIZE,
		file);

	if (result != PACK_HEADER_SIZE)
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
		return BAD_FILE_TYPE_PACK_RESULT;
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

	result = fread(
		&itemCount,
		sizeof(uint64_t),
		1,
		file);

	if (result != 1)
	{
		fclose(file);
		ZSTD_freeDCtx(zstdContext);
		free(packReader);
		return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	if (itemCount == 0)
	{
		fclose(file);
		ZSTD_freeDCtx(zstdContext);
		free(packReader);
		return BAD_DATA_SIZE_PACK_RESULT;
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
uint32_t getPackItemDataSize(
	PackReader packReader,
	uint64_t index)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	return packReader->items[index].info.itemSize;
}
uint32_t getPackItemZipSize(
	PackReader packReader,
	uint64_t index)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	return packReader->items[index].info.zipSize;
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
	void* dataBuffer,
	void* zipBuffer)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	assert(dataBuffer != NULL);

	FILE* file = packReader->file;
	PackItemInfo info = packReader->items[index].info;

	int64_t fileOffset = (int64_t)(info.fileOffset +
		sizeof(PackItemInfo) + info.pathSize);

	int seekResult = seekFile(
		file,
		fileOffset,
		SEEK_SET);

	if (seekResult != 0)
		return FAILED_TO_SEEK_FILE_PACK_RESULT;

	if (info.zipSize > 0)
	{
		if (zipBuffer == NULL)
		{
			uint8_t* zipData = malloc(
				info.zipSize * sizeof(uint8_t));

			if (zipData == NULL)
				return FAILED_TO_ALLOCATE_PACK_RESULT;

			size_t result = fread(
				zipData,
				sizeof(uint8_t),
				info.zipSize,
				file);

			if (result != info.zipSize)
			{
				free(zipData);
				return FAILED_TO_READ_FILE_PACK_RESULT;
			}

			result = ZSTD_decompressDCtx(
				packReader->zstdContext,
				dataBuffer,
				info.itemSize,
				zipData,
				info.zipSize);

			free(zipData);

			if (ZSTD_isError(result) ||
				result != info.itemSize)
			{
				return FAILED_TO_DECOMPRESS_PACK_RESULT;
			}
		}
		else
		{
			size_t result = fread(
				zipBuffer,
				sizeof(uint8_t),
				info.zipSize,
				file);

			if (result != info.zipSize)
				return FAILED_TO_READ_FILE_PACK_RESULT;

			result = ZSTD_decompressDCtx(
				packReader->zstdContext,
				dataBuffer,
				info.itemSize,
				zipBuffer,
				info.zipSize);

			if (ZSTD_isError(result) ||
				result != info.itemSize)
			{
				return FAILED_TO_DECOMPRESS_PACK_RESULT;
			}
		}
	}
	else
	{
		size_t result = fread(
			dataBuffer,
			sizeof(uint8_t),
			info.itemSize,
			file);

		if (result != info.itemSize)
			return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	return SUCCESS_PACK_RESULT;
}

PackResult createPackItemData(
	PackReader packReader,
	uint64_t index,
	uint32_t* _size,
	uint8_t** _data)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	assert(_size != NULL);
	assert(_data != NULL);

	uint32_t size =
		packReader->items[index].info.itemSize;
	uint8_t* data = malloc(
		size * sizeof(uint8_t));

	if (data == NULL)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	PackResult result = readPackItemData(
		packReader,
		index,
		data,
		NULL);

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

inline static void removePackItemFiles(
	uint64_t itemCount,
	PackItem* packItems)
{
	for (uint64_t i = 0; i < itemCount; i++)
		remove(packItems[i].path);
}
PackResult unpackItems(
	const char* packPath,
	uint64_t* _itemCount,
	bool printProgress)
{
	assert(packPath != NULL);
	assert(_itemCount != NULL);

	PackReader packReader;

	PackResult packResult = createPackReader(
		packPath,
		&packReader);

	if (packResult != SUCCESS_PACK_RESULT)
		return packResult;

	uint64_t itemCount = packReader->itemCount;
	PackItem* items = packReader->items;

	uint32_t maxDataSize = 0;
	uint32_t maxZipSize = 0;

	for (uint64_t i = 0; i < itemCount; i++)
	{
		if (items[i].info.itemSize > maxDataSize)
			maxDataSize = items[i].info.itemSize;
		if (items[i].info.zipSize > maxZipSize)
			maxZipSize = items[i].info.zipSize;
	}

	uint8_t* itemData = malloc(
		maxDataSize * sizeof(uint8_t));

	if (itemData == NULL)
	{
		destroyPackReader(packReader);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	uint8_t* zipData = malloc(
		maxZipSize * sizeof(uint8_t));

	if (zipData == NULL)
	{
		free(itemData);
		destroyPackReader(packReader);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	for (uint64_t i = 0; i < itemCount; i++)
	{
		PackItem* item = &items[i];

		if (printProgress == true)
		{
			printf("Unpacking \"%s\" file. ", item->path);
			fflush(stdout);
		}

		uint32_t itemSize = item->info.itemSize;

		packResult = readPackItemData(
			packReader,
			i,
			itemData,
			zipData);

		if (packResult != SUCCESS_PACK_RESULT)
		{
			removePackItemFiles(
				i,
				items);
			free(zipData);
			free(itemData);
			destroyPackReader(packReader);
			return packResult;
		}

		uint8_t pathSize = item->info.pathSize;

		char itemPath[UINT8_MAX + 1];

		memcpy(
			itemPath,
			item->path,
			pathSize);
		itemPath[pathSize] = 0;

		for (uint8_t j = 0; j < pathSize; j++)
		{
			if (itemPath[j] == '/' ||
				itemPath[j] == '\\')
			{
				itemPath[j] = '-';
			}
		}

		FILE* itemFile = openFile(
			itemPath,
			"wb");

		if (itemFile == NULL)
		{
			removePackItemFiles(
				i,
				items);
			free(zipData);
			free(itemData);
			destroyPackReader(packReader);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		size_t result = fwrite(
			itemData,
			sizeof(uint8_t),
			itemSize,
			itemFile);

		fclose(itemFile);

		if (result != itemSize)
		{
			removePackItemFiles(
				i,
				items);
			free(zipData);
			free(itemData);
			destroyPackReader(packReader);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		if (printProgress == true)
		{
			int progress = (int)((float)(i + 1) /
				(float)itemCount * 100.0f);
			printf("[%d%%]\n", progress);
			fflush(stdout);
		}
	}

	free(zipData);
	free(itemData);
	destroyPackReader(packReader);

	*_itemCount = itemCount;
	return SUCCESS_PACK_RESULT;
}
