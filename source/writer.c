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
#include "lz4hc.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct FileItemPath
{
	const char* filePath;
	const char* itemPath;
} FileItemPath;

typedef struct CompressorData
{
	uint8_t* itemData;
	uint8_t* zipData;
	PackItemHeader* itemHeaders;
	void* zipContext;
	FILE* itemFile;
	bool preferSpeed;
} CompressorData;

static void destroyCompressorData(CompressorData* compressor)
{
	if (compressor->itemFile)
		closeFile(compressor->itemFile);
	if (compressor->preferSpeed)
		free(compressor->zipContext);
	else ZSTD_freeCCtx(compressor->zipContext);

	free(compressor->itemHeaders);
	free(compressor->zipData);
	free(compressor->itemData);
}

/**********************************************************************************************************************/
static PackResult writePackItems(FILE* packFile, uint64_t itemCount,
	const FileItemPath* pathPairs, float zipThreshold, bool preferSpeed,
	bool printProgress, OnPackFile onPackFile, void* argument)
{
	assert(packFile != NULL);
	assert(itemCount > 0);
	assert(pathPairs != NULL);

	CompressorData compressor;
	memset(&compressor, 0, sizeof(CompressorData));
	compressor.preferSpeed = preferSpeed;

	uint32_t bufferSize = 1;
	compressor.itemData = malloc(sizeof(uint8_t));
	if (!compressor.itemData)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	compressor.zipData = malloc(sizeof(uint8_t));
	if (!compressor.zipData)
	{
		destroyCompressorData(&compressor);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	compressor.itemHeaders = malloc(itemCount * sizeof(PackItemHeader));
	if (!compressor.itemHeaders)
	{
		destroyCompressorData(&compressor);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	if (preferSpeed)
		compressor.zipContext = malloc(LZ4_sizeofStateHC());
	else compressor.zipContext = ZSTD_createCCtx();

	if (!compressor.zipContext)
	{
		destroyCompressorData(&compressor);
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
			destroyCompressorData(&compressor);
			return BAD_DATA_SIZE_PACK_RESULT;
		}

		compressor.itemFile = openFile(pathPairs[i].filePath, "rb");
		if (!compressor.itemFile)
		{
			destroyCompressorData(&compressor);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		if (seekFile(compressor.itemFile, 0, SEEK_END) != 0)
		{
			destroyCompressorData(&compressor);
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
		}

		uint64_t fileSize = (uint64_t)tellFile(compressor.itemFile);
		if (fileSize > UINT32_MAX)
		{
			destroyCompressorData(&compressor);
			return BAD_DATA_SIZE_PACK_RESULT;
		}

		uint64_t sameDataOffset = UINT64_MAX;
		uint8_t* zipItemData = NULL; uint32_t zipItemSize = 0;

		PackItemHeader header;
		header.dataSize = (uint32_t)fileSize;
		header.pathSize = (uint8_t)pathSize;

		if (header.dataSize > 0)
		{
			if (header.dataSize > bufferSize)
			{
				uint8_t* newBuffer = realloc(compressor.itemData, header.dataSize);
				if (!newBuffer)
				{
					destroyCompressorData(&compressor);
					return FAILED_TO_ALLOCATE_PACK_RESULT;
				}
				compressor.itemData = newBuffer;

				newBuffer = realloc(compressor.zipData, header.dataSize);
				if (!newBuffer)
				{
					destroyCompressorData(&compressor);
					return FAILED_TO_ALLOCATE_PACK_RESULT;
				}
				compressor.zipData = newBuffer;
			}

			if (seekFile(compressor.itemFile, 0, SEEK_SET) != 0)
			{
				destroyCompressorData(&compressor);
				return FAILED_TO_SEEK_FILE_PACK_RESULT;
			}

			size_t result = fread(compressor.itemData, sizeof(uint8_t), header.dataSize, compressor.itemFile);
			closeFile(compressor.itemFile); compressor.itemFile = NULL;

			if (result != header.dataSize)
			{
				destroyCompressorData(&compressor);
				return FAILED_TO_READ_FILE_PACK_RESULT;
			}

			uint32_t isError;
			if (preferSpeed)
			{
				header.zipSize = (uint32_t)LZ4_compress_HC_extStateHC(
					compressor.zipContext, (const char*)compressor.itemData, (char*)compressor.zipData, 
					header.dataSize, header.dataSize - 1, LZ4HC_CLEVEL_MAX);
				isError = header.zipSize == 0;
			}
			else
			{
				result = ZSTD_compressCCtx((ZSTD_CCtx*)compressor.zipContext, compressor.zipData, 
					header.dataSize - 1, compressor.itemData, header.dataSize, ZSTD_maxCLevel());
				header.zipSize = (uint32_t)result;
				isError = ZSTD_isError(result);
			}

			if (isError || (zipThreshold + (double)header.zipSize / (double)header.dataSize > 1.0))
			{
				header.zipSize = 0;
				zipItemData = compressor.zipData;
				zipItemSize = header.dataSize;
			}
			else
			{
				zipItemData = compressor.itemData;
				zipItemSize = header.zipSize;
			}
		
			for (size_t j = 0; j < i; j++)
			{
				PackItemHeader* otherHeader = &compressor.itemHeaders[j];
				if (otherHeader->zipSize != header.zipSize || otherHeader->dataSize != header.dataSize)
					continue;

				if (seekFile(packFile, otherHeader->dataOffset, SEEK_SET) != 0)
				{
					destroyCompressorData(&compressor);
					return FAILED_TO_SEEK_FILE_PACK_RESULT;
				}

				if (fread(zipItemData, sizeof(uint8_t), zipItemSize, packFile) != zipItemSize)
				{
					destroyCompressorData(&compressor);
					return FAILED_TO_READ_FILE_PACK_RESULT;
				}
				if (memcmp(compressor.itemData, compressor.zipData, zipItemSize) == 0)
				{
					sameDataOffset = otherHeader->dataOffset;
					break;
				}
			}
		}

		if (seekFile(packFile, fileOffset, SEEK_SET) != 0)
		{
			destroyCompressorData(&compressor);
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
		}
		
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

		compressor.itemHeaders[i] = header;
		if (fwrite(&header, sizeof(PackItemHeader), 1, packFile) != 1)
		{
			destroyCompressorData(&compressor);
			return FAILED_TO_WRITE_FILE_PACK_RESULT;
		}

		if (fwrite(itemPath, sizeof(char), header.pathSize, packFile) != header.pathSize)
		{
			destroyCompressorData(&compressor);
			return FAILED_TO_WRITE_FILE_PACK_RESULT;
		}

		fileOffset += sizeof(PackItemHeader) + header.pathSize;

		if (header.dataSize > 0 && sameDataOffset == UINT64_MAX)
		{
			zipItemData = header.zipSize > 0 ? compressor.zipData : compressor.itemData;
			if (fwrite(zipItemData, sizeof(uint8_t), zipItemSize, packFile) != zipItemSize)
			{
				destroyCompressorData(&compressor);
				return FAILED_TO_WRITE_FILE_PACK_RESULT;
			}

			fileOffset += zipItemSize;

			if (printProgress)
			{
				rawFileSize += header.dataSize;
				printf("(%u/%u bytes)\n", zipItemSize, header.dataSize);
				fflush(stdout);
			}
		}
		else
		{
			if (printProgress)
			{
				rawFileSize += header.dataSize;
				printf("(0/%u bytes)\n", header.dataSize);
				fflush(stdout);
			}
		}
	}

	destroyCompressorData(&compressor);

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
PackResult packFiles(const char* filePath, uint64_t fileCount, const char** fileItemPaths, uint32_t dataVersion, 
	float zipThreshold, bool preferSpeed, bool printProgress, OnPackFile onPackFile, void* argument)
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
	header.dataVersion = dataVersion;
	header.preferSpeed = preferSpeed ? 1 : 0;
	header._reserved = 0;

	size_t writeResult = fwrite(&header, sizeof(PackHeader), 1, packFile);
	if (writeResult != 1)
	{
		free(pathPairs); closeFile(packFile); remove(filePath);
		return FAILED_TO_WRITE_FILE_PACK_RESULT;
	}

	PackResult packResult = writePackItems(packFile, itemCount, pathPairs, 
		zipThreshold, preferSpeed, printProgress, onPackFile, argument);

	free(pathPairs);
	closeFile(packFile);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		remove(filePath);
		return packResult;
	}

	return SUCCESS_PACK_RESULT;
}