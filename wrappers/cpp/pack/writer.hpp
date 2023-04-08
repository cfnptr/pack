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
#include <exception>
#include <string_view>

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
	 * Pack files to the pack file.
	 * Throws runtime exception on failure.
	 *
	 * packPath - pack file path string.
	 * fileCount - to pack file count.
	 * fileItemPaths - pack file and item path strings.
	 * printProgress - printf reading progress.
	 */
	static void pack(string_view packPath, uint64_t fileCount,
		const char** fileItemPaths, bool printProgress = false)
	{
		auto result = packFiles(packPath.data(),
			fileCount, fileItemPaths, printProgress);
		if (result != SUCCESS_PACK_RESULT)
			throw runtime_error(packResultToString(result));
	}
};

} // pack