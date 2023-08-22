// Copyright 2021-2023 Nikita Fediuchin. All rights reserved.
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
 * Pack files to the pack file.
 * Returns operation Pack result.
 *
 * packPath - pack file path string.
 * fileCount - to pack file count.
 * fileItemPaths - pack file and item path strings.
 * zipThreshold - compression threshold. (0.0 - 1.0 range)
 * printProgress - printf reading progress.
 */
PackResult packFiles(const char* packPath, uint64_t fileCount,
	const char** fileItemPaths, float zipThreshold, bool printProgress);