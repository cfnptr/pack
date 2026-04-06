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

static void printPackerHelp()
{
	/////////////////////////////////////////////////////////////////////////////////////
	printf("Usage: packer [-z, -v, -s] <pack-path> <file-path-1> <item-path-1>...\n"
		"\n"
		"Note that file path and item path may differ!\n"
		"\n"
		"Options:\n"
		"  -z <zipThreshold> Specifies file compression threshold when we just pack file \n"
		"                    without compressing it. It's used for already compressed \n"
		"                    resources like images. Default value is 10. (0% - 100% range)\n"
		"  -v <dataVersion>  Specifies pack file version. It's used to check if we are \n"
		"                    loading correct resources pack for a current game or \n"
		"                    application version. Default value is 0.\n"
		"  -s Use faster decompression algorithm sacrificing resources pack file size.\n"
	);
}

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printPackerHelp();
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
		else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
		{
			printPackerHelp();
			return EXIT_SUCCESS;
		}
		break;
	}

	char* packPath = argv[argOffset++];
	int itemCount = argc - argOffset;

	if (itemCount <= 0 || itemCount % 2 != 0)
	{
		printf("Bad pack file and item count, missing some of the items.\n");
		return EXIT_FAILURE;
	}

	PackResult result = packFiles(packPath, itemCount / 2, (const char**)argv + 
		argOffset, dataVersion, zipThreshold, preferSpeed, true, NULL, NULL);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n", packResultToString(result));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}