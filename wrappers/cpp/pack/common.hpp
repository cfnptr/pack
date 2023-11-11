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
#include "pack/common.h"
}

namespace pack
{

using namespace std;

/*
 * Pack common functions.
 */
class Pack final
{
public:
	/*
	 * Get Pack library version. (MT-Safe)
	 * Returns hardcoded version value.
	 *
	 * major - reference to the major version.
	 * minor - reference to the minor version.
	 * patch - reference to the patch version.
	 */
	static void getLibraryVersion(uint8_t& major,
		uint8_t& minor, uint8_t& patch) noexcept
	{
		getPackLibraryVersion(&major, &minor, &patch);
	}

	/*
	 * Read pack header from the file. (MT-Safe)
	 * Throws runtime exception on failure.
	 *
	 * filePath - file path string.
	 * packHeader - pointer to the pack header.
	 */
	static void readHeader(const filesystem::path& filePath, PackHeader& header)
	{
		auto path = filePath.generic_string();
		auto result = readPackHeader(path.c_str(), &header);
		if (result != SUCCESS_PACK_RESULT)
		{
			throw runtime_error(packResultToString(result) +
				(", path: " + filePath.generic_string()));
		}
	}

	/*
	 * Try to read pack header from the file. (MT-Safe)
	 * Returns false on failure.
	 *
	 * filePath - file path string.
	 * packHeader - pointer to the pack header.
	 */
	static bool tryReadHeader(const filesystem::path& filePath, PackHeader& header)
	{
		auto path = filePath.generic_string();
		auto result = readPackHeader(path.c_str(), &header);
		return result == SUCCESS_PACK_RESULT;
	}
};

} // pack