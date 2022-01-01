// Copyright 2021-2022 Nikita Fediuchin. All rights reserved.
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
#include "pack/defines.h"
#include <stdbool.h>

/*
 * Get pack library version.
 * Returns hardcoded version.
 *
 * majorVersion - major library version.
 * minorVersion - minor library version.
 * patchVersion - patch library version.
 */
void getPackLibraryVersion(
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
PackResult getPackInfo(
	const char* filePath,
	uint8_t* majorVersion,
	uint8_t* minorVersion,
	uint8_t* patchVersion,
	bool* isLittleEndian,
	uint64_t* itemCount);
