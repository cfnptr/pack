#pragma once
#include "pack/defines.h"
#include <stdbool.h>

/*
 * Read pack info from the file.
 * Returns operation pack result.
 *
 * filePath - pack file path string.
 * majorVersion - major pack version.
 * minorVersion - minor pack version.
 * patchVersion - path pack version.
 * isLittleEndian - is pack data little endian.
 * itemCount - stored pack item count.
 */
PackResult getPackInfo(
	const char* filePath,
	uint8_t* majorVersion,
	uint8_t* minorVersion,
	uint8_t* patchVersion,
	bool* isLittleEndian,
	uint64_t* itemCount);
