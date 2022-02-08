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

#include "pack/writer.h"
#include "mpio/file.h"

#include "zstd.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

inline static PackResult writePackItems(
	FILE* packFile,
	uint64_t itemCount,
	char** itemPaths,
	bool printProgress)
{
	assert(packFile);
	assert(itemCount > 0);
	assert(itemPaths);

	uint32_t bufferSize = 1;

	uint8_t* itemData = malloc(
		sizeof(uint8_t));

	if (!itemData)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	uint8_t* zipData = malloc(
		sizeof(uint8_t));

	if (!zipData)
	{
		free(itemData);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	uint64_t totalZipSize = 0, totalRawSize = 0;

	for (uint64_t i = 0; i < itemCount; i++)
	{
		char* itemPath = itemPaths[i];

		if (printProgress)
		{
			printf("Packing \"%s\" file. ", itemPath);
			fflush(stdout);
		}

		size_t pathSize = strlen(itemPath);

		if (pathSize > UINT8_MAX)
		{
			free(zipData);
			free(itemData);
			return BAD_DATA_SIZE_PACK_RESULT;
		}

		FILE* itemFile = openFile(
			itemPath,
			"rb");

		if (!itemFile)
		{
			free(zipData);
			free(itemData);
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		int seekResult = seekFile(
			itemFile,
			0,
			SEEK_END);

		if (seekResult != 0)
		{
			closeFile(itemFile);
			free(zipData);
			free(itemData);
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
		}

		uint64_t itemSize = (uint64_t)tellFile(itemFile);

		if (itemSize == 0 || itemSize > UINT32_MAX)
		{
			closeFile(itemFile);
			free(zipData);
			free(itemData);
			return BAD_DATA_SIZE_PACK_RESULT;
		}

		seekResult = seekFile(
			itemFile,
			0,
			SEEK_SET);

		if (seekResult != 0)
		{
			closeFile(itemFile);
			free(zipData);
			free(itemData);
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
		}

		if (itemSize > bufferSize)
		{
			uint8_t* newBuffer = realloc(
				itemData,
				itemSize * sizeof(uint8_t));

			if (!newBuffer)
			{
				closeFile(itemFile);
				free(zipData);
				free(itemData);
				return FAILED_TO_ALLOCATE_PACK_RESULT;
			}

			itemData = newBuffer;

			newBuffer = realloc(
				zipData,
				itemSize * sizeof(uint8_t));

			if (!newBuffer)
			{
				closeFile(itemFile);
				free(zipData);
				free(itemData);
				return FAILED_TO_ALLOCATE_PACK_RESULT;
			}

			zipData = newBuffer;
		}

		size_t result = fread(
			itemData,
			sizeof(uint8_t),
			itemSize,
			itemFile);

		closeFile(itemFile);

		if (result != itemSize)
		{
			free(zipData);
			free(itemData);
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		size_t zipSize;

		if (itemSize > 1)
		{
			zipSize = ZSTD_compress(
				zipData,
				itemSize - 1,
				itemData,
				itemSize,
				ZSTD_maxCLevel());

			if (ZSTD_isError(zipSize) ||
				zipSize >= itemSize)
			{
				zipSize = 0;
			}
		}
		else
		{
			zipSize = 0;
		}

		int64_t fileOffset = tellFile(packFile);

		PackItemInfo info = {
			(uint32_t)zipSize,
			(uint32_t)itemSize,
			(uint64_t)fileOffset,
			(uint8_t)pathSize,
		};

		result = fwrite(
			&info,
			sizeof(PackItemInfo),
			1,
			packFile);

		if (result != 1)
		{
			free(zipData);
			free(itemData);
			return FAILED_TO_WRITE_FILE_PACK_RESULT;
		}

		result = fwrite(
			itemPath,
			sizeof(char),
			info.pathSize,
			packFile);

		if (result != info.pathSize)
		{
			free(zipData);
			free(itemData);
			return FAILED_TO_WRITE_FILE_PACK_RESULT;
		}

		if (zipSize > 0)
		{
			result = fwrite(
				zipData,
				sizeof(uint8_t),
				zipSize,
				packFile);

			if (result != zipSize)
			{
				free(zipData);
				free(itemData);
				return FAILED_TO_WRITE_FILE_PACK_RESULT;
			}
		}
		else
		{
			result = fwrite(
				itemData,
				sizeof(uint8_t),
				itemSize,
				packFile);

			if (result != itemSize)
			{
				free(zipData);
				free(itemData);
				return FAILED_TO_WRITE_FILE_PACK_RESULT;
			}
		}

		if (printProgress)
		{
			uint32_t zipFileSize = zipSize > 0 ?
				(uint32_t)zipSize : (uint32_t)itemSize;
			uint32_t rawFileSize = (uint32_t)itemSize;

			totalZipSize += zipFileSize;
			totalRawSize += rawFileSize;

			int progress = (int)(
				((float)(i + 1) / (float)itemCount) * 100.0f);

			printf("(%u/%u bytes) [%d%%]\n",
				zipFileSize, rawFileSize, progress);
			fflush(stdout);
		}
	}

	free(zipData);
	free(itemData);

	if (printProgress)
	{
		int compression = (int)((1.0 -
			(double)(totalZipSize) / (double)totalRawSize) * 100.0);
		printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n",
			itemCount, totalZipSize, totalRawSize, compression);
	}

	return SUCCESS_PACK_RESULT;
}
static int comparePackItemPaths(
	const void* _a,
	const void* _b)
{
	// NOTE: a and b should not be NULL!
	// Skipping here assertions for debug build speed.

	char* a = *(char**)_a;
	char* b = *(char**)_b;
	uint8_t al = (uint8_t)strlen(a);
	uint8_t bl = (uint8_t)strlen(b);

	int difference = al - bl;

	if (difference != 0)
		return difference;

	return memcmp(a, b, al);
}
PackResult packFiles(
	const char* filePath,
	uint64_t fileCount,
	const char** filePaths,
	bool printProgress)
{
	assert(filePath);
	assert(fileCount > 0);
	assert(filePaths);

	char** itemPaths = malloc(
		fileCount * sizeof(char*));

	if (!itemPaths)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	uint64_t itemCount = 0;

	for (uint64_t i = 0; i < fileCount; i++)
	{
		bool alreadyAdded = false;

		for (uint64_t j = 0; j < itemCount; j++)
		{
			if (i != j && strcmp(filePaths[i], itemPaths[j]) == 0)
				alreadyAdded = true;
		}

		if (!alreadyAdded)
			itemPaths[itemCount++] = (char*)filePaths[i];
	}

	qsort(itemPaths,
		itemCount,
		sizeof(char*),
		comparePackItemPaths);

	FILE* packFile = openFile(
		filePath,
		"wb");

	if (!packFile)
	{
		free(itemPaths);
		return FAILED_TO_CREATE_FILE_PACK_RESULT;
	}

	char header[PACK_HEADER_SIZE] = {
		'P', 'A', 'C', 'K',
		PACK_VERSION_MAJOR,
		PACK_VERSION_MINOR,
		PACK_VERSION_PATCH,
		!PACK_LITTLE_ENDIAN,
	};

	size_t writeResult = fwrite(
		header,
		sizeof(char),
		PACK_HEADER_SIZE,
		packFile);

	if (writeResult != PACK_HEADER_SIZE)
	{
		free(itemPaths);
		closeFile(packFile);
		remove(filePath);
		return FAILED_TO_WRITE_FILE_PACK_RESULT;
	}

	writeResult = fwrite(
		&itemCount,
		sizeof(uint64_t),
		1,
		packFile);

	if (writeResult != 1)
	{
		free(itemPaths);
		closeFile(packFile);
		remove(filePath);
		return FAILED_TO_WRITE_FILE_PACK_RESULT;
	}

	PackResult packResult = writePackItems(
		packFile,
		itemCount,
		itemPaths,
		printProgress);

	free(itemPaths);
	closeFile(packFile);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		remove(filePath);
		return packResult;
	}

	return SUCCESS_PACK_RESULT;
}
