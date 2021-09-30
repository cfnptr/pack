#include "pack/reader.h"
#include "pack/file.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printf("Usage: <path-to-pack>\n");
		return EXIT_SUCCESS;
	}

	const char* packPath = argv[1];

	uint64_t itemCount;
	uint64_t errorItemIndex;

	PackResult result = decoupleItemPack(
		packPath,
		&itemCount,
		true,
		&errorItemIndex);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("Error at item %llu: %s.\n",
			errorItemIndex,
			packResultToString(result));
		return EXIT_SUCCESS;
	}

	printf("Unpacked %llu items.\n", itemCount);
	return EXIT_SUCCESS;
}
