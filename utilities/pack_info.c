// Copyright 2021-2025 Nikita Fediuchin. All rights reserved.
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

#include "pack/reader.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: pack-info <pack-path>\n");
		return EXIT_FAILURE;
	}

	PackHeader header;
	PackResult result = readPackHeader(argv[1], &header);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n", packResultToString(result));
		return EXIT_FAILURE;
	}

	printf("Pack [v%d.%d.%d]\n\n"
		"Pack information:\n"
		"    Version: %d.%d.%d\n"
		"    Big endian: %s\n"
		"    Item count: %llu\n\n",
		PACK_VERSION_MAJOR, PACK_VERSION_MINOR, PACK_VERSION_PATCH,
		header.versionMajor, header.versionMinor, header.versionPatch,
		header.isBigEndian ? "true" : "false",
		(long long unsigned int)header.itemCount);

	PackReader packReader;
	result = createFilePackReader(argv[1], 0, false, &packReader);
	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n", packResultToString(result));
		return EXIT_FAILURE;
	}

	uint64_t itemCount = getPackItemCount(packReader);
	for (uint64_t i = 0; i < itemCount; ++i)
	{
		printf("Item %llu:\n"
			"    Path: %s\n"
			"    Data size: %u\n"
			"    Zip size: %u\n"
			"    File offset: %llu\n"
			"    Reference: %s\n",
			(long long unsigned int)i,
			getPackItemPath(packReader, i),
			getPackItemDataSize(packReader, i),
			getPackItemZipSize(packReader, i),
			(long long unsigned int)getPackItemFileOffset(packReader, i),
			isPackItemReference(packReader, i) ? "true" : "false");
		fflush(stdout);
	}

	return EXIT_SUCCESS;
}