// Copyright 2021 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <stdio.h>

#if __linux__ || __APPLE__
#define openFile(filePath, mode) \
	fopen(filePath, mode)
#define seekFile(file, offset, whence) \
	fseeko(file, offset, whence)
#define tellFile(file) ftello(file)
#elif _WIN32
inline static FILE* openFile(
	const char* filePath,
	const char* mode)
{
	FILE* file;

	errno_t error = fopen_s(
		&file,
		filePath,
		mode);

	if (error != 0)
		return NULL;

	return file;
}

#define seekFile(file, offset, whence) \
	_fseeki64(file, offset, whence)
#define tellFile(file) _ftelli64(file)
#else
#error Unsupported operating system
#endif

#define closeFile(file) fclose(file)
