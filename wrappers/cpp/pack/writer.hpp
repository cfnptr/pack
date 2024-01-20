// Copyright 2021-2024 Nikita Fediuchin. All rights reserved.
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

/***********************************************************************************************************************
 * @file
 * @brief Pack file writer.
 * @details See the @ref writer.h
 **********************************************************************************************************************/

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

/**
 * @brief Pack writer functions.
 * @details See the @ref writer.h
 */
class Writer final
{
public:
	/**
	 * @brief Packs files to the Pack archive.
	 * @details See the @ref packFiles().
	 *
	 * @param[in] packPath output Pack file path string
	 * @param fileCount file count to pack
	 * @param[in] fileItemPaths pack file and item path string array (file/item, file/item...)
	 * @param zipThreshold compression threshold (0.0 - 1.0 range)
	 * @param printProgress output packing progress to the stdout
	 * @param[in] onPackFile file packing callback, or NULL
	 * @param[in] argument file packing callback argument, or NULL
	 */
	static void pack(const filesystem::path& packPath,
		uint64_t fileCount, const char** fileItemPaths,
		float zipThreshold = 0.1f, bool printProgress = false,
		OnPackFile onPackFile = nullptr, void* argument = nullptr)
	{
		auto path = packPath.generic_string();
		auto result = packFiles(path.c_str(), fileCount, fileItemPaths,
			zipThreshold, printProgress, onPackFile, argument);
		if (result != SUCCESS_PACK_RESULT)
			throw runtime_error(packResultToString(result));
	}
};

} // pack