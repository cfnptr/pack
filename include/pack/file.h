#pragma once
#include <stdio.h>

#if __linux__ || __APPLE__
#define openFile(filePath, mode) \
	fopen(filePath, mode)
#define seekFile(file, offset, whence) \
	fseeko(file, offset, whence)
#define tellFile(file) ftello(file)
#elif _WIN32
#define openFile(filePath, mode) \
{                                \
	FILE* file;                  \
	errno_t error = fopen_s(     \
		file, filePath, mode);   \
	if (error != 0) NULL;        \
	file;                        \
}
#define seekFile(file, offset, whence) \
	_fseeki64(file, offset, whence)
#define tellFile(file) _ftelli64(file)
#else
#error Unsupported operating system
#endif