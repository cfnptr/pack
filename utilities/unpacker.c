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

	uint64_t filesCount;

	PackResult result = unpackFiles(
		argv[1],
		&filesCount,
		true);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n",
			packResultToString(result));
		return EXIT_SUCCESS;
	}

	printf("Unpacked %llu files.\n", filesCount);
	return EXIT_SUCCESS;
}
