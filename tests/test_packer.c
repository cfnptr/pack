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
#include "pack/reader.h"
#include "mpio/file.h"
#include "mpio/directory.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define LOREM_IPSUM "Lorem ipsum dolor sit amet, consectetur adipiscing elit. " \
	"Maecenas aliquet maximus condimentum. Cras et rhoncus eros, tincidunt " \
	"congue nulla. Fusce consequat tristique nisl, nec varius neque finibus " \
	"vitae. Curabitur nec pretium lacus. Mauris egestas commodo tortor vel " \
	"ornare. Quisque condimentum luctus nulla, non ultricies dolor maximus at. " \
	"Duis aliquet efficitur velit quis convallis. Aliquam sapien urna, imperdiet " \
	"non elementum eget, auctor ac enim. Phasellus dignissim quis ante eget " \
	"malesuada. Nam a est at ligula accumsan dignissim. Pellentesque habitant " \
	"morbi tristique senectus et netus et malesuada fames ac turpis egestas. "
#define TEST_FILE_NAME "test.pack"

inline static bool createTestFile(const char* path,
	const void* content, size_t contentLength)
{
	assert(content);

	FILE* file = openFile(path, "w");
	if (!file)
	{
		printf("Failed to open test file.\n");
		return false;
	}

	size_t writeResult = fwrite(content, sizeof(char), contentLength, file);

	if (writeResult != contentLength)
	{
		printf("Failed to write test file.\n");
		closeFile(file);
		return false;
	}

	closeFile(file);
	return true;
}

/**********************************************************************************************************************/
inline static bool testFailedToOpenFile()
{
	PackReader packReader;

	PackResult packResult = createFilePackReader(
		"not_existing_file.pack", 0, false, 1, &packReader);

	if (packResult != FAILED_TO_OPEN_FILE_PACK_RESULT)
	{
		printf("testFailedToOpenFile: "
			"incorrect result. (%d)\n", packResult);
		return false;
	}

	return true;
}

inline static bool testPacker(bool preferSpeed)
{
	const char* files[6] =
	{
		"lorem-ipsum.txt", "lorem-ipsum",
		"_BIN123", "_BIN321_",
		"files/тест", "files/тест"
	};
	const float floats[3] =
	{
		0.0f, 1.0f, -123.456f
	};
	const uint8_t bytes[8] = 
	{
		0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
	};

	createDirectory("files");

	if (!createTestFile(files[0], LOREM_IPSUM, strlen(LOREM_IPSUM)) ||
		!createTestFile(files[2], floats, sizeof(floats)) ||
		!createTestFile(files[4], bytes, sizeof(bytes)))
	{
		remove(files[0]); remove(files[2]); remove(files[4]);
		return false;
	}
	
	PackResult packResult = packFiles(TEST_FILE_NAME, 3, 
		files, 123, 0.1f, preferSpeed, false, NULL, NULL);
	remove(files[0]); remove(files[2]); remove(files[4]);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		printf("testPacker: incorrect result. "
			"(%s)\n", packResultToString(packResult));
		return false;
	}

	PackReader packReader;
	packResult = createFilePackReader(TEST_FILE_NAME, 123, false, 1, &packReader);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		printf("testPacker: incorrect result. "
			"(%s)\n", packResultToString(packResult));
		return false;
	}
	if (getPackItemCount(packReader) != 3)
	{
		printf("testPacker: incorrect item count.");
		return false;
	}

	uint64_t itemIndex;
	if (!getPackItemIndex(packReader, "lorem-ipsum", &itemIndex))
	{
		printf("testPacker: item not found.");
		return false;
	}

	uint32_t itemSize = getPackItemDataSize(packReader, itemIndex);
	char* loremIpsum = malloc(itemSize + 1);
	packResult = readPackItemData(packReader, itemIndex, (uint8_t*)loremIpsum, 0);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		printf("testPacker: incorrect result. "
			"(%s)\n", packResultToString(packResult));
		free(loremIpsum);
		return false;
	}

	loremIpsum[itemSize] = '\0';
	if (strcmp(LOREM_IPSUM, loremIpsum) != 0)
	{
		printf("testPacker: bad item data.");
		free(loremIpsum);
		return false;
	}
	free(loremIpsum);

	if (!getPackItemIndex(packReader, "_BIN321_", &itemIndex))
	{
		printf("testPacker: item not found.");
		return false;
	}
	if (getPackItemDataSize(packReader, itemIndex) != sizeof(floats))
	{
		printf("testPacker: bad item data size.");
		return false;
	}

	float floatData[3];
	packResult = readPackItemData(packReader, itemIndex, (uint8_t*)floatData, 0);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		printf("testPacker: incorrect result. "
			"(%s)\n", packResultToString(packResult));
		return false;
	}
	if (memcmp(floats, floatData, sizeof(floats)) != 0)
	{
		printf("testPacker: bad item data.");
		free(loremIpsum);
		return false;
	}

	if (!getPackItemIndex(packReader, "files/тест", &itemIndex))
	{
		printf("testPacker: item not found.");
		return false;
	}

	destroyPackReader(packReader);
	return true;
}

int main()
{
	bool result = testFailedToOpenFile();
	result &= testPacker(false);
	result &= testPacker(true);
	remove(TEST_FILE_NAME);
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}