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

#include "pack/common.h"
#include "mpio/file.h"

#include <assert.h>

void getPackLibraryVersion(uint8_t* major, uint8_t* minor, uint8_t* patch)
{
	assert(major);
	assert(minor);
	assert(patch);
	*major = PACK_VERSION_MAJOR;
	*minor = PACK_VERSION_MINOR;
	*patch = PACK_VERSION_PATCH;
}

PackResult readPackHeader(const char* filePath, PackHeader* _header)
{
	assert(filePath);
	assert(_header);

	FILE* file = openFile(filePath, "rb");
	if (!file)
		return FAILED_TO_OPEN_FILE_PACK_RESULT;

	PackHeader header;
	size_t result = fread(&header, sizeof(PackHeader), 1, file);
	closeFile(file);

	if (result != 1)
		return FAILED_TO_READ_FILE_PACK_RESULT;
	if (header.magic != PACK_HEADER_MAGIC)
		return BAD_FILE_TYPE_PACK_RESULT;

	*_header = header;
	return SUCCESS_PACK_RESULT;
}