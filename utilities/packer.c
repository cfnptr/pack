// Copyright 2021-2026 Nikita Fediuchin. All rights reserved.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pack/writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printf("Usage: packer [-z, -v, -s] <pack-path> <file-path-1> <item-path-1>...\n");
		return EXIT_FAILURE;
	}

	float zipThreshold = 0.1f;
	uint32_t dataVersion = 0;
	int argOffset = 1;
	bool preferSpeed = false;
	
	while (true)
	{
		char* arg = argv[argOffset];
		if (strcmp(arg, "-z") == 0)
		{
			int zipPercents = atoi(argv[argOffset + 1]);
			if (zipPercents < 0 || zipPercents > 100)
			{
				printf("Bad zip threshold value, should be in range 0 - 100 percents.\n");
				return EXIT_FAILURE;
			}

			zipThreshold = (float)zipPercents * 0.01f;
			argOffset += 2;
			continue;
		}
		else if (strcmp(arg, "-v") == 0)
		{
			long long version = atoll(argv[argOffset + 1]);
			if (version < 0 && version > UINT32_MAX)
			{
				printf("Bad data version value, should be in range 0 - UINT32_MAX.\n");
				return EXIT_FAILURE;
			}

			dataVersion = (uint32_t)version;
			argOffset += 2;
			continue;
		}
		else if (strcmp(arg, "-s") == 0)
		{
			preferSpeed = true;
			argOffset += 1;
			continue;
		}
		break;
	}

	char* packPath = argv[argOffset++];

	if ((argc - argOffset) % 2 != 0)
	{
		printf("Bad pack file and item count, missing some of the items.\n");
		return EXIT_FAILURE;
	}

	PackResult result = packFiles(packPath, (argc - argOffset) / 2, (const char**)argv + argOffset, 
		dataVersion, zipThreshold, preferSpeed, true, NULL, NULL);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n", packResultToString(result));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}