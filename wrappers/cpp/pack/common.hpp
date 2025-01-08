// Copyright 2021-2025 Nikita Fediuchin. All rights reserved.
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
 * @brief Common Pack library functions.
 * @details See the @ref common.h
 **********************************************************************************************************************/

#pragma once
#include "pack/error.hpp"
#include <filesystem>

extern "C"
{
#include "pack/common.h"
}

namespace pack
{

/**
 * @brief Common Pack functions.
 * @details See the @ref common.h
 */
class Common final
{
public:
	/**
	 * @brief Returns Pack library hardcoded version. (MT-Safe)
	 * @details See the @ref getPackLibraryVersion().
	 *
	 * @param[out] major reference to the uint32_t major version
	 * @param[out] minor reference to the uint32_t minor version
	 * @param[out] patch reference to the uint32_t patch version
	 */
	static void getLibraryVersion(uint8_t& major,
		uint8_t& minor, uint8_t& patch) noexcept
	{
		getPackLibraryVersion(&major, &minor, &patch);
	}

	/**
	 * @brief Reads Pack header from the file. (MT-Safe)
	 * @details See the @ref readPackHeader().
	 *
	 * @param[in] filePath target file path string
	 * @param[out] header reference to the @ref PackHeader structure
	 *
	 * @throw Error with a @ref PackResult string on failure.
	 */
	static void readHeader(const filesystem::path& filePath, PackHeader& header)
	{
		auto path = filePath.generic_string();
		auto result = readPackHeader(path.c_str(), &header);
		if (result != SUCCESS_PACK_RESULT)
			throw Error(packResultToString(result) + (", path: " + filePath.generic_string()));
	}

	/**
	 * @brief Tries to read Pack header from the file. (MT-Safe)
	 * @details See the @ref readPackHeader().
	 *
	 * @param[in] filePath target file path string
	 * @param[out] header reference to the @ref PackHeader structure
	 *
	 * @return True on success and writes header data, otherwise false.
	 */
	static bool tryReadHeader(const filesystem::path& filePath, PackHeader& header) noexcept
	{
		auto path = filePath.generic_string();
		auto result = readPackHeader(path.c_str(), &header);
		return result == SUCCESS_PACK_RESULT;
	}
};

} // pack