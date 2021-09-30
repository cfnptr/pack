#define PACK_LOG_WRITE_PROGRESS 1
#include "pack/writer.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printf("Usage: <path-to-pack> <path-to-item>...\n");
		return EXIT_SUCCESS;
	}

	const char* packPath = argv[1];
	const char** itemPaths = (const char**)argv + 2;

	uint64_t errorItemIndex;

	PackResult result = createItemPack(
		packPath,
		argc - 2,
		itemPaths,
		true,
		&errorItemIndex);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("Error at item %llu: %s.\n",
			errorItemIndex,
			packResultToString(result));
	}

	printf("Created pack of %d items.\n", argc - 2);
	return EXIT_SUCCESS;
}
