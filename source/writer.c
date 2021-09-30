#include "pack/writer.h"
#include "pack/file.h"

#include "zstd.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

inline static PackResult writePackItems(
	FILE* packFile,
	uint64_t itemCount,
	const char** itemPaths,
	uint64_t* errorItemIndex)
{
	for (uint64_t i = 0; i < itemCount; i++)
	{
		const char* itemPath = itemPaths[i];

		FILE* itemFile = openFile(
			itemPath,
			"rb");

		if (itemFile == NULL)
		{
			*errorItemIndex = i;
			return FAILED_TO_OPEN_FILE_PACK_RESULT;
		}

		int seekResult = seekFile(
			itemFile,
			0,
			SEEK_END);

		if (seekResult != 0)
		{
			fclose(itemFile);
			*errorItemIndex = i;
			return FAILED_TO_SEEK_FILE_PACK_RESULT;
		}

		uint64_t itemSize = tellFile(itemFile);

		if (itemSize == 0)
		{
			fclose(itemFile);
			*errorItemIndex = i;
			return ZERO_DATA_FILE_SIZE_PACK_RESULT;
		}

		uint8_t* itemData = malloc(
			itemSize * sizeof(uint8_t));

		if (itemData == NULL)
		{
			fclose(itemFile);
			*errorItemIndex = i;
			return FAILED_TO_ALLOCATE_PACK_RESULT;
		}

		size_t result = fread(
			itemData,
			sizeof(uint8_t),
			itemSize,
			itemFile);

		fclose(itemFile);

		if (result != itemSize)
		{
			free(itemData);
			*errorItemIndex = i;
			return FAILED_TO_READ_FILE_PACK_RESULT;
		}

		uint8_t* zipData = malloc(
			itemSize * sizeof(uint8_t));

		if (zipData == NULL)
		{
			free(itemData);
			*errorItemIndex = i;
			return FAILED_TO_ALLOCATE_PACK_RESULT;
		}

		size_t zipSize = ZSTD_compress(
			zipData,
			itemSize,
			itemData,
			itemSize,
			ZSTD_maxCLevel());

		if (ZSTD_isError(zipSize))
			zipSize = 0;

		PackItemInfo info = {
			zipSize,
			itemSize,
			tellFile(packFile),
			strlen(itemPath),
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
			*errorItemIndex = i;
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
			*errorItemIndex = i;
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
				*errorItemIndex = i;
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
				*errorItemIndex = i;
				return FAILED_TO_WRITE_FILE_PACK_RESULT;
			}
		}

		free(zipData);
		free(itemData);
	}

	return SUCCESS_PACK_RESULT;
}
PackResult createItemPack(
	const char* filePath,
	uint64_t itemCount,
	const char** itemPaths,
	uint64_t* errorItemIndex)
{
	assert(filePath != NULL);
	assert(itemCount != 0);
	assert(itemPaths != NULL);
	assert(errorItemIndex != NULL);

	FILE* packFile = openFile(
		filePath,
		"wb");

	if (packFile == NULL)
		return FAILED_TO_OPEN_FILE_PACK_RESULT;

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
		fclose(packFile);
		remove(filePath);
		return FAILED_TO_WRITE_FILE_PACK_RESULT;
	}

	PackResult packResult = writePackItems(
		packFile,
		itemCount,
		itemPaths,
		errorItemIndex);

	fclose(packFile);
	return packResult;
}
