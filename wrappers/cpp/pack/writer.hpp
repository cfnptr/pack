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
#include <string>
#include <exception>
#include <filesystem>

extern "C"
{
#include "pack/writer.h"
}

namespace pack
{

using namespace std;

/*
 * Pack writer functions.
 */
class Writer final
{
public:
	/*
	 * Pack files to the pack file. (MT-Safe)
	 * Throws runtime exception on failure.
	 *
	 * packPath - pack file path string.
	 * fileCount - to pack file count.
	 * fileItemPaths - pack file and item path strings.
	 * zipThreshold - compression threshold. (0.0 - 1.0 range)
	 * printProgress - printf reading progress.
	 */
	static void pack(const filesystem::path& packPath,
		uint64_t fileCount, const char** fileItemPaths,
		float zipThreshold = 0.1f, bool printProgress = false)
	{
		auto path = packPath.generic_string();
		auto result = packFiles(path.c_str(), fileCount,
			fileItemPaths, zipThreshold, printProgress);
		if (result != SUCCESS_PACK_RESULT)
			throw runtime_error(packResultToString(result));
	}
};

} // pack