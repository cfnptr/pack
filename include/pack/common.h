#pragma once
#include "pack/defines.h"
#include <stdbool.h>

// TODO: add lib version getter

/*
 * Get pack library version.
 * Returns hardcoded version.
 *
 * majorVersion - major library version.
 * minorVersion - minor library version.
 * patchVersion - patch library version.
 */
PACK_API void getPackLibraryVersion(
	uint8_t* majorVersion,
	uint8_t* minorVersion,
	uint8_t* patchVersion);

/*
 * Read pack info from the file.
 * Returns operation pack result.
 *
 * filePath - pack file path string.
 * majorVersion - major pack version.
 * minorVersion - minor pack version.
 * patchVersion - patch pack version.
 * isLittleEndian - is pack data little endian.
 * itemCount - stored pack item count.
 */
PACK_API PackResult getPackInfo(
	const char* filePath,
	uint8_t* majorVersion,
	uint8_t* minorVersion,
	uint8_t* patchVersion,
	bool* isLittleEndian,
	uint64_t* itemCount);
