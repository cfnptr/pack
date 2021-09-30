#pragma once

typedef struct PackWriter* PackWriter;

PackWriter createPackWriter(
	const char* packFilePath);
void destroyPackWriter(
	PackWriter packWriter);
