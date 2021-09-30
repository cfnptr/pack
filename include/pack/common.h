#pragma once
#include "pack/defines.h"
#include <stdbool.h>

PackResult getPackInfo(
	const char* filePath,
	uint8_t* majorVersion,
	uint8_t* minorVersion,
	uint8_t* patchVersion,
	bool* isLittleEndian,
	uint64_t* itemCount);
