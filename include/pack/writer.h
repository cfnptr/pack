#pragma once
#include "pack/defines.h"

PackResult createItemPack(
	const char* filePath,
	uint64_t itemCount,
	const char** itemPaths,
	uint64_t* errorItemIndex);
