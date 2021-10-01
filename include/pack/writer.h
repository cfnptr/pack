#pragma once
#include "pack/defines.h"
#include <stdbool.h>

PackResult packItems(
	const char* filePath,
	uint64_t itemCount,
	const char** itemPaths,
	bool printProgress);
