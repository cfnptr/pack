#include "pack/common.h"
#include "pack/reader.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: pack-info <path-to-pack>\n");
		return EXIT_SUCCESS;
	}

	uint8_t majorVersion;
	uint8_t minorVersion;
	uint8_t patchVersion;
	bool isLittleEndian;
	uint64_t itemCount;

	PackResult result = getPackInfo(
		argv[1],
		&majorVersion,
		&minorVersion,
		&patchVersion,
		&isLittleEndian,
		&itemCount);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n",
			packResultToString(result));
		return EXIT_SUCCESS;
	}

	printf("Pack library [v%d.%d.%d]\n\n"
		"Pack information:\n"
		"    Version: %d.%d.%d.\n"
		"    Little endian: %s.\n"
		"    Item count: %llu.\n\n",
		PACK_VERSION_MAJOR,
		PACK_VERSION_MINOR,
		PACK_VERSION_PATCH,
		majorVersion,
		minorVersion,
		patchVersion,
		isLittleEndian ? "TRUE" : "FALSE",
		itemCount);

	PackReader packReader;

	result = createPackReader(
		argv[1],
		&packReader);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n",
			packResultToString(result));
		return EXIT_SUCCESS;
	}

	itemCount = getPackItemCount(packReader);

	for (uint64_t i = 0; i < itemCount; ++i)
	{
		printf("Item %llu:\n"
			"    Path: %s.\n"
			"    Size: %u.\n",
			i,
			getPackItemPath(packReader, i),
			getPackItemDataSize(packReader, i));
	}

	return EXIT_SUCCESS;
}
