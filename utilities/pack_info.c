#include "pack/common.h"

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

	printf("Pack information:\n"
		"    Version: %d.%d.%d\n"
		"    Little endian: %s\n"
		"    Item count: %llu\n",
		majorVersion,
		minorVersion,
		patchVersion,
		isLittleEndian ? "TRUE" : "FALSE",
		itemCount);
	return EXIT_SUCCESS;
}
