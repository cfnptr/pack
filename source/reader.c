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

#include "pack/reader.h"
#include "mpio/file.h"

#include "zstd.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct PackItem
{
	PackItemInfo info;
	char* path;
} PackItem;
struct PackReader_T
{
	ZSTD_DCtx* zstdContext;
	FILE* file;
	uint64_t itemCount;
	PackItem* items;
	uint8_t* dataBuffer;
	uint8_t* zipBuffer;
	uint32_t dataSize;
	uint32_t zipSize;
	PackItem searchItem;
};

inline static void destroyPackItems(
	uint64_t itemCount,
	PackItem* items)
{
	assert(itemCount == 0 ||
		(itemCount > 0 && items));

	for (uint64_t i = 0; i < itemCount; i++)
		free(items[i].path);
	free(items);
}
inline static PackResult createPackItems(
	FILE* packFile,
	uint64_t itemCount,
	PackItem** _items)
{
	assert(packFile);
	assert(itemCount > 0);
	assert(_items);

	PackItem* items = malloc(
		itemCount * sizeof(PackItem));

	if (!items)
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

		if (info.dataSize == 0 ||
			info.pathSize == 0)
		{
			destroyPackItems(i, items);
			return BAD_DATA_SIZE_PACK_RESULT;
		}

		char* path = malloc(
			(info.pathSize + 1) * sizeof(char));

		if (!path)
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
			info.zipSize : info.dataSize;

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
PackResult createFilePackReader(
	const char* filePath,
	uint32_t dataBufferCapacity,
	PackReader* packReader)
{
	assert(filePath);
	assert(packReader);

	PackReader packReaderInstance = calloc(1,
		sizeof(PackReader_T));

	if (!packReaderInstance)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	packReaderInstance->zipBuffer = NULL;
	packReaderInstance->zipSize = 0;

	ZSTD_DCtx* zstdContext = ZSTD_createDCtx();

	if (!zstdContext)
	{
		destroyPackReader(packReaderInstance);
		return FAILED_TO_CREATE_ZSTD_PACK_RESULT;
	}

	packReaderInstance->zstdContext = zstdContext;

	FILE* file = openFile(filePath, "rb");

	if (!file)
	{
		destroyPackReader(packReaderInstance);
		return FAILED_TO_OPEN_FILE_PACK_RESULT;
	}

	packReaderInstance->file = file;

	char header[PACK_HEADER_SIZE];

	size_t result = fread(
		header,
		sizeof(char),
		PACK_HEADER_SIZE,
		file);

	if (result != PACK_HEADER_SIZE)
	{
		destroyPackReader(packReaderInstance);
		return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	if (header[0] != 'P' ||
		header[1] != 'A' ||
		header[2] != 'C' ||
		header[3] != 'K')
	{
		destroyPackReader(packReaderInstance);
		return BAD_FILE_TYPE_PACK_RESULT;
	}

	if (header[4] != PACK_VERSION_MAJOR ||
		header[5] != PACK_VERSION_MINOR)
	{
		destroyPackReader(packReaderInstance);
		return BAD_FILE_VERSION_PACK_RESULT;
	}

	// Skipping PATCH version check

	if (header[7] != !PACK_LITTLE_ENDIAN)
	{
		destroyPackReader(packReaderInstance);
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
		destroyPackReader(packReaderInstance);
		return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	if (itemCount == 0)
	{
		destroyPackReader(packReaderInstance);
		return BAD_DATA_SIZE_PACK_RESULT;
	}

	PackItem* items;

	PackResult packResult = createPackItems(
		file,
		itemCount,
		&items);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		destroyPackReader(packReaderInstance);;
		return packResult;
	}

	packReaderInstance->itemCount = itemCount;
	packReaderInstance->items = items;

	uint8_t* dataBuffer;

	if (dataBufferCapacity > 0)
	{
		dataBuffer = malloc(
			dataBufferCapacity * sizeof(uint8_t));

		if (!dataBuffer)
		{
			destroyPackReader(packReaderInstance);
			return FAILED_TO_ALLOCATE_PACK_RESULT;
		}
	}
	else
	{
		dataBuffer = NULL;
	}

	packReaderInstance->dataBuffer = dataBuffer;
	packReaderInstance->dataSize = dataBufferCapacity;

	*packReader = packReaderInstance;
	return SUCCESS_PACK_RESULT;
}
void destroyPackReader(PackReader packReader)
{
	if (!packReader)
		return;

	free(packReader->dataBuffer);
	free(packReader->zipBuffer);
	destroyPackItems(
		packReader->itemCount,
		packReader->items);
	if (packReader->file)
		closeFile(packReader->file);
	size_t result = ZSTD_freeDCtx(
		packReader->zstdContext);
	if (result != 0) abort();
	free(packReader);
}

uint64_t getPackItemCount(PackReader packReader)
{
	assert(packReader);
	return packReader->itemCount;
}

static int comparePackItems(
	const void* _a,
	const void* _b)
{
	// NOTE: a and b should not be NULL!
	// Skipping here assertions for debug build speed.

	const PackItem* a = _a;
	const PackItem* b = _b;

	int difference =
		(int)a->info.pathSize -
		(int)b->info.pathSize;

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
	assert(packReader);
	assert(path);
	assert(index);
	assert(strlen(path) <= UINT8_MAX);

	PackItem* searchItem =
		&packReader->searchItem;

	searchItem->info.pathSize =
		(uint8_t)strlen(path);
	searchItem->path = (char*)path;

	PackItem* item = bsearch(
		searchItem,
		packReader->items,
		packReader->itemCount,
		sizeof(PackItem),
		comparePackItems);

	if (!item)
		return false;

	*index = item - packReader->items;
	return true;
}

uint32_t getPackItemDataSize(
	PackReader packReader,
	uint64_t index)
{
	assert(packReader);
	assert(index < packReader->itemCount);
	return packReader->items[index].info.dataSize;
}

const char* getPackItemPath(
	PackReader packReader,
	uint64_t index)
{
	assert(packReader);
	assert(index < packReader->itemCount);
	return packReader->items[index].path;
}

PackResult readPackItemData(
	PackReader packReader,
	uint64_t index,
	const uint8_t** data,
	uint32_t* size)
{
	assert(packReader);
	assert(index < packReader->itemCount);
	assert(data);
	assert(size);

	PackItemInfo info = packReader->items[index].info;
	uint8_t* dataBuffer = packReader->dataBuffer;

	if (dataBuffer)
	{
		if (info.dataSize > packReader->dataSize)
		{
			dataBuffer = realloc(dataBuffer,
				info.dataSize * sizeof(uint8_t));

			if (!dataBuffer)
				return FAILED_TO_ALLOCATE_PACK_RESULT;

			packReader->dataBuffer = dataBuffer;
			packReader->dataSize = info.dataSize;
		}
	}
	else
	{
		dataBuffer = malloc(
			info.dataSize * sizeof(uint8_t));

		if (!dataBuffer)
			return FAILED_TO_ALLOCATE_PACK_RESULT;

		packReader->dataBuffer = dataBuffer;
		packReader->dataSize = info.dataSize;
	}

	uint8_t* zipBuffer = packReader->zipBuffer;

	if (zipBuffer)
	{
		if (info.zipSize > packReader->zipSize)
		{
			zipBuffer = realloc(zipBuffer,
				info.zipSize * sizeof(uint8_t));

			if (!zipBuffer)
				return FAILED_TO_ALLOCATE_PACK_RESULT;

			packReader->zipBuffer = zipBuffer;
			packReader->zipSize = info.zipSize;
		}
	}
	else
	{
		if (info.zipSize > 0)
		{
			zipBuffer = malloc(
				info.zipSize * sizeof(uint8_t));

			if (!zipBuffer)
				return FAILED_TO_ALLOCATE_PACK_RESULT;

			packReader->zipBuffer = zipBuffer;
			packReader->zipSize = info.zipSize;
		}
	}

	FILE* file = packReader->file;

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
			info.dataSize,
			zipBuffer,
			info.zipSize);

		if (ZSTD_isError(result) ||
			result != info.dataSize)
		{
			return FAILED_TO_DECOMPRESS_PACK_RESULT;
		}
	}
	else
	{
		size_t result = fread(
			dataBuffer,
			sizeof(uint8_t),
			info.dataSize,
			file);

		if (result != info.dataSize)
			return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	*data = dataBuffer;
	*size = info.dataSize;
	return SUCCESS_PACK_RESULT;
}

PackResult readPackPathItemData(
	PackReader packReader,
	const char* path,
	const uint8_t** data,
	uint32_t* size)
{
	assert(packReader);
	assert(path);
	assert(data);
	assert(size);
	assert(strlen(path) <= UINT8_MAX);

	uint64_t index;

	if (!getPackItemIndex(packReader,
		path, &index))
	{
		return FAILED_TO_GET_ITEM_PACK_RESULT;
	}

	return readPackItemData(
		packReader,
		index,
		data,
		size);
}

void freePackReaderBuffers(
	PackReader packReader)
{
	assert(packReader);
	free(packReader->dataBuffer);
	free(packReader->zipBuffer);
	packReader->dataBuffer = NULL;
	packReader->zipBuffer = NULL;
}

inline static void removePackItemFiles(
	uint64_t itemCount,
	PackItem* packItems)
{
	assert(itemCount == 0 ||
		(itemCount > 0 && packItems));

	for (uint64_t i = 0; i < itemCount; i++)
		remove(packItems[i].path);
}
PackResult unpackFiles(
	const char* filePath,
	bool printProgress)
{
	assert(filePath);

	PackReader packReader;

	PackResult packResult = createFilePackReader(
		filePath,
		0,
		&packReader);

	if (packResult != SUCCESS_PACK_RESULT)
		return packResult;

	uint64_t totalRawSize = 0, totalZipSize = 0;

	uint64_t itemCount = packReader->itemCount;
	PackItem* items = packReader->items;

	for (uint64_t i = 0; i < itemCount; i++)
	{
		PackItem* item = &items[i];

		if (printProgress)
		{
			printf("Unpacking \"%s\" file. ", item->path);
			fflush(stdout);
		}

		const uint8_t* dataBuffer;
		uint32_t dataSize;

		packResult = readPackItemData(
			packReader,
			i,
			&dataBuffer,
			&dataSize);

		if (packResult != SUCCESS_PACK_RESULT)
		{
			removePackItemFiles(
				i,
				items);
			destroyPackReader(packReader);
			return packResult;
		}

		uint8_t pathSize = item->info.pathSize;

		char itemPath[UINT8_MAX + 1];

		memcpy(itemPath, item->path, pathSize);
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

		if (!itemFile)
		{
			removePackItemFiles(
				i,
				items);
			destroyPackReader(packReader);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		size_t result = fwrite(
			dataBuffer,
			sizeof(uint8_t),
			dataSize,
			itemFile);

		closeFile(itemFile);

		if (result != dataSize)
		{
			removePackItemFiles(
				i,
				items);
			destroyPackReader(packReader);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		if (printProgress)
		{
			uint32_t rawFileSize = item->info.dataSize;
			uint32_t zipFileSize = item->info.zipSize > 0 ?
				item->info.zipSize : item->info.dataSize;

			totalRawSize += rawFileSize;
			totalZipSize += zipFileSize;

			int progress = (int)(
				((float)(i + 1) / (float)itemCount) * 100.0f);

			printf("(%u/%u bytes) [%d%%]\n",
				rawFileSize, zipFileSize, progress);
			fflush(stdout);
		}
	}

	destroyPackReader(packReader);

	if (printProgress)
	{
		printf("Unpacked %llu files. (%llu/%llu bytes)\n",
			itemCount, totalRawSize, totalZipSize);
	}

	return SUCCESS_PACK_RESULT;
}
