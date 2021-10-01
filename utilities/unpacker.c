#include "pack/reader.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: unpacker <path-to-pack>\n");
		return EXIT_SUCCESS;
	}

	const char* packPath = argv[1];

	uint64_t itemCount;

	PackResult result = unpackItems(
		packPath,
		&itemCount,
		true);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n",
			packResultToString(result));
		return EXIT_SUCCESS;
	}

	printf("Unpacked %llu items.\n", itemCount);
	return EXIT_SUCCESS;
}
