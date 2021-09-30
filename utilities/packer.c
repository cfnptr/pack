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

	PackResult result = createItemPack(
		packPath,
		argc - 2,
		itemPaths,
		true);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n",
			packResultToString(result));
		return EXIT_SUCCESS;
	}

	printf("Packed %d items.\n", argc - 2);
	return EXIT_SUCCESS;
}
