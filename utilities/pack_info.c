// Copyright 2021-2022 Nikita Fediuchin. All rights reserved.
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

#include "pack/common.h"
#include "pack/reader.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: pack-info <path-to-pack>\n");
		return EXIT_FAILURE;
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
		return EXIT_FAILURE;
	}

	printf("Pack library [v%d.%d.%d]\n\n"
		"Pack information:\n"
		"    Version: %d.%d.%d.\n"
		"    Little endian: %s.\n"
		"    Item count: %llu.\n\n",
		PACK_VERSION_MAJOR,
		PACK_VERSION_MINOR,
		PACK_VERSION_PATCH,
		majorVersion,
		minorVersion,
		patchVersion,
		isLittleEndian ? "TRUE" : "FALSE",
		(long long unsigned int)itemCount);

	PackReader packReader;

	result = createFilePackReader(
		argv[1],
		0,
		&packReader);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n",
			packResultToString(result));
		return EXIT_FAILURE;
	}

	itemCount = getPackItemCount(packReader);

	for (uint64_t i = 0; i < itemCount; ++i)
	{
		printf("Item %llu:\n"
			"    Path: %s.\n"
			"    Size: %u.\n",
			(long long unsigned int)i,
			getPackItemPath(packReader, i),
			getPackItemDataSize(packReader, i));
	}

	return EXIT_SUCCESS;
}
