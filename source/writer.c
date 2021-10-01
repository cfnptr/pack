#include "pack/writer.h"
#include "pack/file.h"

#include "zstd.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// TODO: use zip stream instead of malloc

inline static PackResult writePackItems(
	FILE* packFile,
	uint64_t itemCount,
	const char** itemPaths,
	bool printProgress)
{
	uint32_t bufferSize = 1;

	uint8_t* itemData = malloc(
		sizeof(uint8_t));

	if (itemData == NULL)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	uint8_t* zipData = malloc(
		sizeof(uint8_t));

	if (zipData == NULL)
	{
		free(itemData);
		return FAILED_TO_ALLOCATE_PACK_RESULT;
	}

	for (uint64_t i = 0; i < itemCount; i++)
	{
		const char* itemPath = itemPaths[i];

		if (printProgress == true)
			printf("Packing \"%s\" file. ", itemPath);

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

		if (itemFile == NULL)
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
			fclose(itemFile);
			free(zipData);
			free(itemData);
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
		}

		uint32_t itemSize = tellFile(itemFile);

		if (itemSize == 0)
		{
			fclose(itemFile);
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
			fclose(itemFile);
			free(zipData);
			free(itemData);
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
		}

		if (itemSize > bufferSize)
		{
			uint8_t* newBuffer = realloc(
				itemData,
				itemSize);

			if (newBuffer == NULL)
			{
				fclose(itemFile);
				free(zipData);
				free(itemData);
				return FAILED_TO_ALLOCATE_PACK_RESULT;
			}

			itemData = newBuffer;

			newBuffer = realloc(
				zipData,
				itemSize);

			if (newBuffer == NULL)
			{
				fclose(itemFile);
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

		fclose(itemFile);

		if (result != itemSize)
		{
			free(zipData);
			free(itemData);
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		size_t zipSize = ZSTD_compress(
			zipData,
			itemSize,
			itemData,
			itemSize,
			ZSTD_maxCLevel());

		if (ZSTD_isError(zipSize) ||
			zipSize >= itemSize)
		{
			zipSize = 0;
		}

		PackItemInfo info = {
			zipSize,
			itemSize,
			tellFile(packFile),
			pathSize,
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

		if (zipSize != 0)
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

		if (printProgress == true)
		{
			int progress = (int)((float)(i + 1) /
				(float)itemCount * 100.0f);
			printf("[%d%%]\n", progress);
		}
	}

	free(zipData);
	free(itemData);
	return SUCCESS_PACK_RESULT;
}
static int comparePackItemPaths(
	const void* _a,
	const void* _b)
{
	const char* a = *(const char**)_a;
	const char* b = *(const char**)_b;
	uint8_t al = (uint8_t)strlen(a);
	uint8_t bl = (uint8_t)strlen(b);

	int difference = al - bl;

	if (difference != 0)
		return difference;

	return memcmp(a, b, al);
}
PackResult packItems(
	const char* filePath,
	uint64_t itemCount,
	const char** _itemPaths,
	bool printProgress)
{
	assert(filePath != NULL);
	assert(itemCount != 0);
	assert(_itemPaths != NULL);

	const char** itemPaths = malloc(
		itemCount * sizeof(const char*));

	if (itemPaths == NULL)
		return FAILED_TO_ALLOCATE_PACK_RESULT;

	for (uint64_t i = 0; i < itemCount; i++)
		itemPaths[i] = _itemPaths[i];

	qsort(
		itemPaths,
		itemCount,
		sizeof(const char*),
		comparePackItemPaths);

	FILE* packFile = openFile(
		filePath,
		"wb");

	if (packFile == NULL)
	{
		free(itemPaths);
		return FAILED_TO_OPEN_FILE_PACK_RESULT;
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
		fclose(packFile);
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
		fclose(packFile);
		remove(filePath);
		return FAILED_TO_WRITE_FILE_PACK_RESULT;
	}

	PackResult packResult = writePackItems(
		packFile,
		itemCount,
		itemPaths,
		printProgress);

	free(itemPaths);
	fclose(packFile);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		remove(filePath);
		return packResult;
	}

	return SUCCESS_PACK_RESULT;
}
