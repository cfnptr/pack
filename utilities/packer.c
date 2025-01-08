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

#include "pack/writer.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc <= 2 || (argc - 2) % 2 != 0)
	{
		printf("Usage: packer <pack-path> <file-path-1> <item-path-1>...\n");
		return EXIT_FAILURE;
	}

	// TODO: add zip threshold option to the command line arguments.

	PackResult result = packFiles(argv[1], (argc - 2) / 2,
		(const char**)argv + 2, 0.1f, true, NULL, NULL);

	if (result != SUCCESS_PACK_RESULT)
	{
		printf("\nError: %s.\n", packResultToString(result));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}