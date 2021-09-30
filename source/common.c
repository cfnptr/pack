#include "pack/common.h"
#include "pack/file.h"

#include <assert.h>

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

	if (result != sizeof(uint64_t))
		return FAILED_TO_READ_FILE_PACK_RESULT;

	*majorVersion = header[4];
	*minorVersion = header[5];
	*patchVersion = header[6];
	*isLittleEndian = !header[7];
	*_itemCount = itemCount;
	return SUCCESS_PACK_RESULT;
}
