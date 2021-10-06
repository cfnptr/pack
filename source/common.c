// Copyright 2021 Nikita Fediuchin. All rights reserved.
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
#include "pack/file.h"

#include <assert.h>

void getPackLibraryVersion(
	uint8_t* majorVersion,
	uint8_t* minorVersion,
	uint8_t* patchVersion)
{
	assert(majorVersion != NULL);
	assert(minorVersion != NULL);
	assert(patchVersion != NULL);

	*majorVersion = PACK_VERSION_MAJOR;
	*minorVersion = PACK_VERSION_MINOR;
	*patchVersion = PACK_VERSION_PATCH;
}

PackResult getPackInfo(
	const char* filePath,
	uint8_t* majorVersion,
	uint8_t* minorVersion,
	uint8_t* patchVersion,
	bool* isLittleEndian,
	uint64_t* _itemCount)
{
	assert(filePath != NULL);
	assert(majorVersion != NULL);
	assert(minorVersion != NULL);
	assert(patchVersion != NULL);
	assert(isLittleEndian != NULL);
	assert(_itemCount != NULL);

	FILE* file = openFile(
		filePath,
		"rb");

	if (file == NULL)
		return FAILED_TO_OPEN_FILE_PACK_RESULT;

	char header[PACK_HEADER_SIZE];

	size_t result = fread(
		header,
		sizeof(char),
		PACK_HEADER_SIZE,
		file);

	if (result != PACK_HEADER_SIZE)
	{
		fclose(file);
		return FAILED_TO_READ_FILE_PACK_RESULT;
	}

	if (header[0] != 'P' ||
		header[1] != 'A' ||
		header[2] != 'C' ||
		header[3] != 'K')
	{
		fclose(file);
		return BAD_FILE_TYPE_PACK_RESULT;
	}

	uint64_t itemCount;

	result = fread(
		&itemCount,
		sizeof(uint64_t),
		1,
		file);

	fclose(file);

	if (result != 1)
		return FAILED_TO_READ_FILE_PACK_RESULT;

	*majorVersion = header[4];
	*minorVersion = header[5];
	*patchVersion = header[6];
	*isLittleEndian = !header[7];
	*_itemCount = itemCount;
	return SUCCESS_PACK_RESULT;
}
