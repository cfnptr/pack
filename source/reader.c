// Copyright 2021-2024 Nikita Fediuchin. All rights reserved.
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
#include "mpio/directory.h"
#include "zstd.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct PackItem
{
	PackItemHeader header;
	char* path;
} PackItem;
struct PackReader_T
{
	ZSTD_DCtx** zstdContexts;
	FILE** files;
	uint64_t itemCount;
	PackItem* items;
	uint32_t threadCount;
};

static void destroyPackItems(uint64_t itemCount, PackItem* items)
{
	assert(itemCount == 0 || (itemCount > 0 && items != NULL));
	for (uint64_t i = 0; i < itemCount; i++) free(items[i].path);
	free(items);
}
static PackResult createPackItems(FILE* packFile,
	uint64_t itemCount, PackItem** _items)
{
	assert(packFile != NULL);
	assert(itemCount > 0);
	assert(_items != NULL);

	PackItem* items = malloc(itemCount * sizeof(PackItem));
	if (!items)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	for (uint64_t i = 0; i < itemCount; i++)
	{
		PackItemHeader header;
		if (fread(&header, sizeof(PackItemHeader), 1, packFile) != 1)
		{
			destroyPackItems(i, items);
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		if (header.dataSize == 0 || header.pathSize == 0 || header.dataOffset == 0)
		{
			destroyPackItems(i, items);
			return BAD_DATA_SIZE_PACK_RESULT;
		}

		char* path = malloc(header.pathSize + 1);
		if (!path)
		{
			destroyPackItems(i, items);
			return FAILED_TO_ALLOCATE_PACK_RESULT;
		}

		if (fread(path, sizeof(char), header.pathSize, packFile) != header.pathSize)
		{
			destroyPackItems(i, items);
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		path[header.pathSize] = 0;

		if (!header.isReference)
		{
			int64_t fileOffset = header.zipSize > 0 ? header.zipSize : header.dataSize;
			int seekResult = seekFile(packFile, fileOffset, SEEK_CUR);

			if (seekResult != 0)
			{
				destroyPackItems(i, items);
				return FAILED_TO_SEEK_FILE_PACK_RESULT;
			}
		}

		PackItem item;
		item.header = header;
		item.path = path;
		items[i] = item;
	}

	*_items = items;
	return SUCCESS_PACK_RESULT;
}

/**********************************************************************************************************************/
PackResult createFilePackReader(const char* filePath,
	bool isResourcesDirectory, uint32_t threadCount, PackReader* packReader)
{
	assert(filePath != NULL);
	assert(threadCount > 0);
	assert(packReader != NULL);

	PackReader packReaderInstance = calloc(1, sizeof(PackReader_T));
	if (!packReaderInstance)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	char* path;

#if __APPLE__
	if (isResourcesDirectory)
	{
		char* resourcesDirectory = getResourcesDirectory();
		if (!resourcesDirectory)
		{
			destroyPackReader(packReaderInstance);
			return FAILED_TO_GET_DIRECTORY_PACK_RESULT;
		}

		size_t filePathLength = strlen(filePath);
		size_t resourcesPathLength = strlen(resourcesDirectory);
		size_t pathLength = filePathLength + resourcesPathLength + 2;

		path = malloc(pathLength);
		if (!path)
		{
			destroyPackReader(packReaderInstance);
			return FAILED_TO_ALLOCATE_PACK_RESULT;
		}

		memcpy(path, resourcesDirectory, resourcesPathLength);
		path[resourcesPathLength] = '/';
		memcpy(path + resourcesPathLength + 1, filePath, filePathLength);
		path[resourcesPathLength + filePathLength + 1] = '\0';
		free(resourcesDirectory);
	}
	else
	{
		path = (char*)filePath;
	}
#else
	path = (char*)filePath;
#endif

	FILE** files = calloc(threadCount, sizeof(FILE*));
	if (!files)
	{
#if __APPLE__
		if (isResourcesDirectory)
			free(path);
#endif
		destroyPackReader(packReaderInstance);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	packReaderInstance->files = files;

	ZSTD_DCtx** zstdContexts = calloc(threadCount, sizeof(ZSTD_DCtx*));
	if (!zstdContexts)
	{
#if __APPLE__
		if (isResourcesDirectory)
			free(path);
#endif
		destroyPackReader(packReaderInstance);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	packReaderInstance->zstdContexts = zstdContexts;

	for (uint32_t i = 0; i < threadCount; i++)
	{
		FILE* file = openFile(path, "rb");
		if (!file)
		{
#if __APPLE__
			if (isResourcesDirectory)
				free(path);
#endif
			destroyPackReader(packReaderInstance);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		files[i] = file;

		ZSTD_DCtx* zstdContext = ZSTD_createDCtx();
		if (!zstdContext)
		{
			destroyPackReader(packReaderInstance);
			return FAILED_TO_CREATE_ZSTD_PACK_RESULT;
		}

		zstdContexts[i] = zstdContext;
	}

#if __APPLE__
	if (isResourcesDirectory)
		free(path);
#endif

	PackHeader header;
	FILE* file = files[0];

	if (fread(&header, sizeof(PackHeader), 1, file) != 1)
	{
		destroyPackReader(packReaderInstance);
		return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	if (header.magic != PACK_HEADER_MAGIC)
	{
		destroyPackReader(packReaderInstance);
		return BAD_FILE_TYPE_PACK_RESULT;
	}
	if (header.versionMajor != PACK_VERSION_MAJOR ||
		header.versionMinor != PACK_VERSION_MINOR)
	{
		destroyPackReader(packReaderInstance);
		return BAD_FILE_VERSION_PACK_RESULT;
	}

	// Skipping PATCH version check

	if (header.isBigEndian != !PACK_LITTLE_ENDIAN)
	{
		destroyPackReader(packReaderInstance);
		return BAD_FILE_ENDIANNESS_PACK_RESULT;
	}

	if (header.itemCount == 0)
	{
		destroyPackReader(packReaderInstance);
		return BAD_DATA_SIZE_PACK_RESULT;
	}

	PackItem* items;
	PackResult packResult = createPackItems(file, header.itemCount, &items);
	if (packResult != SUCCESS_PACK_RESULT)
	{
		destroyPackReader(packReaderInstance);;
		return packResult;
	}

	packReaderInstance->itemCount = header.itemCount;
	packReaderInstance->items = items;
	packReaderInstance->threadCount = threadCount;

	*packReader = packReaderInstance;
	return SUCCESS_PACK_RESULT;
}
void destroyPackReader(PackReader packReader)
{
	if (!packReader)
		return;

	destroyPackItems(packReader->itemCount, packReader->items);

	FILE** files = packReader->files;
	ZSTD_DCtx** zstdContexts = packReader->zstdContexts;
	uint32_t threadCount = packReader->threadCount;

	for (uint32_t i = 0; i < threadCount; i++)
	{
		if (ZSTD_freeDCtx(zstdContexts[i]) != 0) abort();
		if (files[i])
			closeFile(files[i]);
	}
	
	free(packReader->zstdContexts);
	free(packReader->files);
	free(packReader);
}

/**********************************************************************************************************************/
uint64_t getPackItemCount(PackReader packReader)
{
	assert(packReader != NULL);
	return packReader->itemCount;
}

static int comparePackItems(const void* _a, const void* _b)
{
	// NOTE: a and b should not be NULL!
	// Skipping here assertions for debug build speed.

	const PackItem* a = _a;
	const PackItem* b = _b;
	int difference = (int)a->header.pathSize - (int)b->header.pathSize;
	if (difference != 0)
		return difference;
	return memcmp(a->path, b->path, a->header.pathSize);
}
bool getPackItemIndex(PackReader packReader, const char* path, uint64_t* index)
{
	assert(packReader != NULL);
	assert(path != NULL);
	assert(index != NULL);
	assert(strlen(path) <= UINT8_MAX);

	PackItem searchItem;
	searchItem.header.pathSize = (uint8_t)strlen(path);
	searchItem.path = (char*)path;

	PackItem* item = bsearch(&searchItem,
		packReader->items, packReader->itemCount,
		sizeof(PackItem), comparePackItems);
	if (!item)
		return false;

	*index = item - packReader->items;
	return true;
}

uint32_t getPackItemDataSize(PackReader packReader, uint64_t index)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	return packReader->items[index].header.dataSize;
}

uint32_t getPackItemZipSize(PackReader packReader, uint64_t index)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	return packReader->items[index].header.zipSize;
}

PackResult readPackItemData(PackReader packReader,
	uint64_t itemIndex, uint8_t* buffer, uint32_t threadIndex)
{
	assert(packReader);
	assert(itemIndex < packReader->itemCount);
	assert(buffer != NULL);
	assert(threadIndex < packReader->threadCount);

	PackItemHeader header = packReader->items[itemIndex].header;
	FILE* file = packReader->files[threadIndex];
	int seekResult = seekFile(file, header.dataOffset, SEEK_SET);
	if (seekResult != 0)
		return FAILED_TO_SEEK_FILE_PACK_RESULT;

	if (header.zipSize > 0)
	{
		uint8_t* zipBuffer = malloc(header.zipSize);
		if (!zipBuffer)
			return FAILED_TO_ALLOCATE_PACK_RESULT;

		if (fread(zipBuffer, sizeof(uint8_t), header.zipSize, file) != header.zipSize)
		{
			free(zipBuffer);
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		size_t result = ZSTD_decompressDCtx(packReader->zstdContexts[threadIndex],
			buffer, header.dataSize, zipBuffer, header.zipSize);
		free(zipBuffer);

		if (ZSTD_isError(result) || result != header.dataSize)
			return FAILED_TO_DECOMPRESS_PACK_RESULT;
	}
	else
	{
		size_t result = fread(buffer, sizeof(uint8_t), header.dataSize, file);
		if (result != header.dataSize)
			return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	return SUCCESS_PACK_RESULT;
}

/**********************************************************************************************************************/
uint64_t getPackItemFileOffset(PackReader packReader, uint64_t index)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	return packReader->items[index].header.dataOffset;
}

bool isPackItemReference(PackReader packReader, uint64_t index)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	return packReader->items[index].header.isReference;
}

const char* getPackItemPath(PackReader packReader, uint64_t index)
{
	assert(packReader != NULL);
	assert(index < packReader->itemCount);
	return packReader->items[index].path;
}

void** const getPackZstdContexts(PackReader packReader)
{
	assert(packReader != NULL);
	return (void** const)packReader->zstdContexts;
}

/**********************************************************************************************************************/
static void removePackItemFiles(uint64_t itemCount, PackItem* packItems)
{
	assert(itemCount == 0 || (itemCount > 0 && packItems != NULL));
	for (uint64_t i = 0; i < itemCount; i++) remove(packItems[i].path);
}
PackResult unpackFiles(const char* filePath, bool printProgress)
{
	assert(filePath != NULL);

	PackReader packReader; // TODO: do this from multiple threads.
	PackResult packResult = createFilePackReader(filePath, false, 1, &packReader);
	if (packResult != SUCCESS_PACK_RESULT)
		return packResult;

	uint64_t rawFileSize = 0;
	uint64_t fileOffset = sizeof(PackHeader);
	uint64_t itemCount = packReader->itemCount;
	PackItem* items = packReader->items;

	for (uint64_t i = 0; i < itemCount; i++)
	{
		const PackItem* item = &items[i];
		if (printProgress)
		{
			int progress = (int)(((float)(i + 1) / (float)itemCount) * 100.0f);
			const char* spacing;
			if (progress < 10)
				spacing = "  ";
			else if (progress < 100)
				spacing = " ";
			else
				spacing = "";
			printf("[%s%d%%] Unpacking file %s ", spacing, progress, item->path);
			fflush(stdout);
		}

		uint32_t dataSize = getPackItemDataSize(packReader, i);
		uint8_t* data = malloc(dataSize);
		if (!data)
		{
			removePackItemFiles(i, items);
			destroyPackReader(packReader);
			return FAILED_TO_ALLOCATE_PACK_RESULT;
		}

		packResult = readPackItemData(packReader, i, data, 0);
		if (packResult != SUCCESS_PACK_RESULT)
		{
			free(data);
			removePackItemFiles(i, items);
			destroyPackReader(packReader);
			return packResult;
		}

		uint8_t pathSize = item->header.pathSize;
		char itemPath[UINT8_MAX + 1];
		memcpy(itemPath, item->path, pathSize);
		itemPath[pathSize] = 0;

		for (uint8_t j = 0; j < pathSize; j++)
		{
			if (itemPath[j] == '/' || itemPath[j] == '\\')
				itemPath[j] = '-';
		}

		FILE* itemFile = openFile(itemPath, "wb");
		if (!itemFile)
		{
			free(data);
			removePackItemFiles(i, items);
			destroyPackReader(packReader);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		size_t result = fwrite(data, sizeof(uint8_t), dataSize, itemFile);
		free(data);
		closeFile(itemFile);

		if (result != dataSize)
		{
			removePackItemFiles(i, items);
			destroyPackReader(packReader);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		if (printProgress)
		{
			rawFileSize += dataSize;
			fileOffset += sizeof(PackItemHeader) + pathSize;
			
			if (item->header.isReference)
			{
				printf("(0/%u bytes)\n", dataSize);
			}
			else
			{
				uint32_t zipItemSize = item->header.zipSize > 0 ?
					item->header.zipSize : item->header.dataSize;
				fileOffset += zipItemSize;
				printf("(%u/%u bytes)\n", zipItemSize, dataSize);
			}
			
			fflush(stdout);
		}
	}

	destroyPackReader(packReader);

	if (printProgress)
	{
		int compression = (int)((1.0 -
			(double)(fileOffset) / (double)rawFileSize) * 100.0);
		printf("Unpacked %llu files. (%llu/%llu bytes, %d%% saved)\n",
			(long long unsigned int)itemCount,
			(long long unsigned int)fileOffset,
			(long long unsigned int)rawFileSize,
			compression);
	}

	return SUCCESS_PACK_RESULT;
}