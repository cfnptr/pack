#include "pack/writer.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printf("Usage: packer <path-to-pack> <path-to-item>...\n");
		return EXIT_SUCCESS;
	}

	PackResult result = packItems(
		argv[1],
		argc - 2,
		(const char**)argv + 2,
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
