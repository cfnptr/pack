#pragma once
#include "pack/defines.h"
#include <stdbool.h>

/*
 * Pack files to the pack.
 * Returns operation pack result.
 *
 * packPath - pack file path string.
 * fileCount - to pack file count.
 * filePaths - pack item path strings.
 * printProgress - printf reading progress.
 */
PACK_API PackResult packFiles(
	const char* packPath,
	uint64_t fileCount,
	const char** filePaths,
	bool printProgress);
