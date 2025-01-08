// Copyright 2021-2025 Nikita Fediuchin. All rights reserved.
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

#include "pack/writer.h"
#include "mpio/file.h"

#include "zstd.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct FileItemPath
{
	const char* filePath;
	const char* itemPath;
} FileItemPath;

static PackResult writePackItems(FILE* packFile, uint64_t itemCount,
	const FileItemPath* pathPairs, float zipThreshold,
	bool printProgress, OnPackFile onPackFile, void* argument)
{
	assert(packFile != NULL);
	assert(itemCount > 0);
	assert(pathPairs != NULL);

	uint32_t bufferSize = 1;
	uint8_t* itemData = malloc(sizeof(uint8_t));
	if (!itemData)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	uint8_t* zipData = malloc(sizeof(uint8_t));
	if (!zipData)
	{
		free(itemData);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	PackItemHeader* itemHeaders = malloc(itemCount * sizeof(PackItemHeader));
	if (!itemHeaders)
	{
		free(zipData); free(itemData);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	uint64_t rawFileSize = 0;
	uint64_t fileOffset = sizeof(PackHeader);

	for (uint64_t i = 0; i < itemCount; i++)
	{
		const char* itemPath = pathPairs[i].itemPath;
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
			printf("[%s%d%%] Packing file %s ", spacing, progress, itemPath);
			fflush(stdout);
		}

		if (onPackFile)
			onPackFile(i, argument);

		size_t pathSize = strlen(itemPath);
		if (pathSize > UINT8_MAX)
		{
			free(itemHeaders); free(zipData); free(itemData);
			return BAD_DATA_SIZE_PACK_RESULT;
		}

		FILE* itemFile = openFile(pathPairs[i].filePath, "rb");
		if (!itemFile)
		{
			free(itemHeaders); free(zipData); free(itemData);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		if (seekFile(itemFile, 0, SEEK_END) != 0)
		{
			closeFile(itemFile);
			free(itemHeaders); free(zipData); free(itemData);
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
		}

		uint64_t itemSize = (uint64_t)tellFile(itemFile);
		if (itemSize > UINT32_MAX)
		{
			closeFile(itemFile);
			free(itemHeaders); free(zipData); free(itemData);
			return BAD_DATA_SIZE_PACK_RESULT;
		}

		uint64_t sameDataOffset = UINT64_MAX;
		size_t zipSize = 0, zipItemSize = 0; uint8_t* zipItemData = NULL;

		if (itemSize > 0)
		{
			if (itemSize > bufferSize)
			{
				uint8_t* newBuffer = realloc(itemData, itemSize);
				if (!newBuffer)
				{
					closeFile(itemFile);
					free(itemHeaders); free(zipData); free(itemData);
					return FAILED_TO_ALLOCATE_PACK_RESULT;
				}

				itemData = newBuffer;
				newBuffer = realloc(zipData, itemSize);
				if (!newBuffer)
				{
					closeFile(itemFile);
					free(itemHeaders); free(zipData); free(itemData);
					return FAILED_TO_ALLOCATE_PACK_RESULT;
				}

				zipData = newBuffer;
			}

			if (seekFile(itemFile, 0, SEEK_SET) != 0)
			{
				closeFile(itemFile);
				free(itemHeaders); free(zipData); free(itemData);
				return FAILED_TO_SEEK_FILE_PACK_RESULT;
			}

			size_t result = fread(itemData, sizeof(uint8_t), itemSize, itemFile);
			closeFile(itemFile);

			if (result != itemSize)
			{
				free(itemHeaders); free(zipData); free(itemData);
				return FAILED_TO_READ_FILE_PACK_RESULT;
			}

			zipSize = ZSTD_compress(zipData, itemSize - 1, itemData, itemSize, ZSTD_maxCLevel());

			if (ZSTD_isError(zipSize) || (zipThreshold + (double)zipSize / (double)itemSize > 1.0))
			{
				zipSize = 0;
				zipItemData = zipData;
				zipItemSize = itemSize;
			}
			else
			{
				zipItemData = itemData;
				zipItemSize = zipSize;
			}
		
			for (size_t j = 0; j < i; j++)
			{
				PackItemHeader* header = &itemHeaders[j];
				if (header->zipSize != zipSize || header->dataSize != itemSize)
					continue;

				if (seekFile(packFile, header->dataOffset, SEEK_SET) != 0)
				{
					free(itemHeaders); free(zipData); free(itemData);
					return FAILED_TO_SEEK_FILE_PACK_RESULT;
				}

				if (fread(zipItemData, sizeof(uint8_t), zipItemSize, packFile) != zipItemSize)
				{
					free(itemHeaders); free(zipData); free(itemData);
					return FAILED_TO_READ_FILE_PACK_RESULT;
				}
				if (memcmp(itemData, zipData, zipItemSize) == 0)
				{
					sameDataOffset = header->dataOffset;
					break;
				}
			}
		}

		if (seekFile(packFile, fileOffset, SEEK_SET) != 0)
		{
			free(itemHeaders); free(zipData); free(itemData);
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
		}

		PackItemHeader header;
		header.zipSize = (uint32_t)zipSize;
		header.dataSize = (uint32_t)itemSize;
		header.pathSize = (uint32_t)pathSize;
		
		if (sameDataOffset == UINT64_MAX)
		{
			header.dataOffset = (uint64_t)(fileOffset +
				sizeof(PackItemHeader) + pathSize);
			header.isReference = 0;
		}
		else
		{
			header.dataOffset = sameDataOffset;
			header.isReference = 1;
		}

		itemHeaders[i] = header;
		if (fwrite(&header, sizeof(PackItemHeader), 1, packFile) != 1)
		{
			free(itemHeaders); free(zipData); free(itemData);
			return FAILED_TO_WRITE_FILE_PACK_RESULT;
		}

		if (fwrite(itemPath, sizeof(char), header.pathSize, packFile) != header.pathSize)
		{
			free(itemHeaders); free(zipData); free(itemData);
			return FAILED_TO_WRITE_FILE_PACK_RESULT;
		}

		fileOffset += sizeof(PackItemHeader) + header.pathSize;

		if (itemSize > 0 && sameDataOffset == UINT64_MAX)
		{
			zipItemData = zipSize > 0 ? zipData : itemData;
			if (fwrite(zipItemData, sizeof(uint8_t), zipItemSize, packFile) != zipItemSize)
			{
				free(itemHeaders); free(zipData); free(itemData);
				return FAILED_TO_WRITE_FILE_PACK_RESULT;
			}

			fileOffset += zipItemSize;

			if (printProgress)
			{
				rawFileSize += itemSize;
				printf("(%u/%u bytes)\n", (uint32_t)zipItemSize, (uint32_t)itemSize);
				fflush(stdout);
			}
		}
		else
		{
			if (printProgress)
			{
				rawFileSize += itemSize;
				printf("(0/%u bytes)\n", (uint32_t)itemSize);
				fflush(stdout);
			}
		}
	}

	free(itemHeaders);
	free(zipData);
	free(itemData);

	if (printProgress)
	{
		int compression = (int)((1.0 -
			(double)(fileOffset) / (double)rawFileSize) * 100.0);
		printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n",
			(long long unsigned int)itemCount,
			(long long unsigned int)fileOffset,
			(long long unsigned int)rawFileSize,
			compression);
	}

	return SUCCESS_PACK_RESULT;
}
static int comparePackPathPairs(const void* _a, const void* _b)
{
	// NOTE: a and b should not be NULL!
	// Skipping here assertions for debug build speed.
	FileItemPath* a = (FileItemPath*)_a;
	FileItemPath* b = (FileItemPath*)_b;
	uint8_t al = (uint8_t)strlen(a->itemPath);
	uint8_t bl = (uint8_t)strlen(b->itemPath);
	int difference = al - bl;
	if (difference != 0)
		return difference;
	return memcmp(a->itemPath, b->itemPath, al);
}

/**********************************************************************************************************************/
PackResult packFiles(const char* filePath, uint64_t fileCount,
	const char** fileItemPaths, float zipThreshold,
	bool printProgress, OnPackFile onPackFile, void* argument)
{
	assert(filePath != NULL);
	assert(fileCount > 0);
	assert(fileItemPaths != NULL);

	FileItemPath* pathPairs = malloc(fileCount * sizeof(FileItemPath));
	if (!pathPairs)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	uint64_t itemCount = 0;
	for (uint64_t i = 0; i < fileCount; i++)
	{
		bool alreadyAdded = false;
		for (uint64_t j = 0; j < itemCount; j++)
		{
			if (i != j && strcmp(fileItemPaths[i * 2], pathPairs[j].filePath) == 0)
				alreadyAdded = true;
		}

		if (!alreadyAdded)
		{
			FileItemPath pathPair;
			pathPair.filePath = fileItemPaths[i * 2];
			pathPair.itemPath = fileItemPaths[i * 2 + 1];
			pathPairs[itemCount++] = pathPair;
		}
	}

	qsort(pathPairs, itemCount, sizeof(FileItemPath), comparePackPathPairs);

	FILE* packFile = openFile(filePath, "w+b");
	if (!packFile)
	{
		free(pathPairs);
		return FAILED_TO_CREATE_FILE_PACK_RESULT;
	}

	PackHeader header;
	header.magic = PACK_HEADER_MAGIC;
	header.versionMajor = PACK_VERSION_MAJOR;
	header.versionMinor = PACK_VERSION_MINOR;
	header.versionPatch = PACK_VERSION_PATCH;
	header.isBigEndian = !PACK_LITTLE_ENDIAN;
	header.itemCount = itemCount;

	size_t writeResult = fwrite(&header, sizeof(PackHeader), 1, packFile);
	if (writeResult != 1)
	{
		free(pathPairs);
		closeFile(packFile);
		remove(filePath);
		return FAILED_TO_WRITE_FILE_PACK_RESULT;
	}

	PackResult packResult = writePackItems(packFile, itemCount,
		pathPairs, zipThreshold, printProgress, onPackFile, argument);

	free(pathPairs);
	closeFile(packFile);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		remove(filePath);
		return packResult;
	}

	return SUCCESS_PACK_RESULT;
}