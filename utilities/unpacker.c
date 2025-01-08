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
		printf("Usage: unpacker <pack-path>\n");
		return EXIT_FAILURE;
	}

	PackResult result = unpackFiles(argv[1], true);
	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n", packResultToString(result));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}