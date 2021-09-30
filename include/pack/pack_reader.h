#pragma once
#include "pack/defines.h"
#include <stdint.h>

typedef struct PackReader* PackReader;

PackResult createPackReader(
	const char* filePath,
	PackReader* packReader);
void destroyPackReader(
	PackReader packReader);

uint64_t getPackItemCount(
	PackReader packReader);
