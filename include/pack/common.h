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
 * @brief Common Pack library functions.
 **********************************************************************************************************************/

#pragma once
#include "pack/defines.h"
#include <stdbool.h>

#if PACK_LITTLE_ENDIAN
/**
 * @brief Pack file header magic number.
 */
#define PACK_HEADER_MAGIC (('K' << 24) | ('C' << 16) | ('A' << 8) | 'P')
#else
/**
 * @brief Pack file header magic number.
 */
#define PACK_HEADER_MAGIC (('P' << 24) | ('A' << 16) | ('C' << 8) | 'K')
#endif

/***********************************************************************************************************************
 * @brief Pack file header structure.
 *
 * @details
 * Each Pack file begins with a header that contains information about the library and system 
 * used for packing the files. It also contains the total number of files inside the archive.
 */
typedef struct PackHeader
{
	uint32_t magic;
	uint8_t versionMajor;
	uint8_t versionMinor;
	uint8_t versionPatch;
	uint8_t isBigEndian;
	uint64_t itemCount;
} PackHeader;

/***********************************************************************************************************************
 * @brief Pack item header structure.
 *
 * @details
 * Contains information about the packed file inside the archive. This includes the size of its path, 
 * the size of compressed data, whether data are compressed, and the location of the data within the archive file.
 */
typedef struct PackItemHeader
{
	uint32_t zipSize;
	uint32_t dataSize;
	uint8_t pathSize : 8;
	uint8_t isReference : 1;
	uint64_t dataOffset : 55;
} PackItemHeader;

/***********************************************************************************************************************
 * @brief Pack result codes.
 * @enum
 */
typedef enum PackResult_T
{
	SUCCESS_PACK_RESULT = 0,
	FAILED_TO_ALLOCATE_PACK_RESULT = 1,
	FAILED_TO_CREATE_ZSTD_PACK_RESULT = 2,
	FAILED_TO_CREATE_FILE_PACK_RESULT = 3,
	FAILED_TO_OPEN_FILE_PACK_RESULT = 4,
	FAILED_TO_WRITE_FILE_PACK_RESULT = 5,
	FAILED_TO_READ_FILE_PACK_RESULT = 6,
	FAILED_TO_SEEK_FILE_PACK_RESULT = 7,
	FAILED_TO_GET_DIRECTORY_PACK_RESULT = 8,
	FAILED_TO_DECOMPRESS_PACK_RESULT = 9,
	FAILED_TO_GET_ITEM_PACK_RESULT = 10,
	BAD_DATA_SIZE_PACK_RESULT = 11,
	BAD_FILE_TYPE_PACK_RESULT = 12,
	BAD_FILE_VERSION_PACK_RESULT = 13,
	BAD_FILE_ENDIANNESS_PACK_RESULT = 14,
	PACK_RESULT_COUNT = 15
} PackResult_T;
/**
 * @brief Pack result code type.
 */
typedef uint8_t PackResult;

/***********************************************************************************************************************
 * @brief Returns Pack library hardcoded version. (MT-Safe)
 *
 * @details
 * Useful in cases when we load a dynamic Pack library (.dll) or 
 * use it from another programming language. (ex. C#, Java)
 *
 * @param[out] major pointer to the uint32_t major version
 * @param[out] minor pointer to the uint32_t minor version
 * @param[out] patch pointer to the uint32_t patch version
 */
void getPackLibraryVersion(uint8_t* major, uint8_t* minor, uint8_t* patch);

/***********************************************************************************************************************
 * @brief Reads Pack header from the file. (MT-Safe)
 *
 * @details
 * We can retrieve information about the package without creating an instance and loading the paths of all packed files.
 *
 * @param[in] filePath target file path string
 * @param[out] packHeader pointer to the @ref PackHeader structure
 *
 * @return The @ref PackResult code.
 *
 * @retval SUCCESS_PACK_RESULT - successful operation
 * @retval FAILED_TO_OPEN_FILE_PACK_RESULT - the files doesn't exist
 * @retval BAD_FILE_TYPE_PACK_RESULT - file header has bad magic number
 */
PackResult readPackHeader(const char* filePath, PackHeader* header);

/***********************************************************************************************************************
 * @brief Pack result code string array.
 */
static const char* const packResultStrings[PACK_RESULT_COUNT] = {
	"Success",
	"Failed to allocate",
	"Failed to create ZSTD",
	"Failed to create file",
	"Failed to open file",
	"Failed to write file",
	"Failed to read file",
	"Failed to seek file",
	"Failed to get directory",
	"Failed to decompress",
	"Failed to get item",
	"Bad data size",
	"Bad file type",
	"Bad file version",
	"Bad file endianness",
};

/**
 * @brief Returns Pack result code as a string.
 * @param result pack result code
 * @result Pack result code string. Or "Unknown PACK result" if out of range.
 */
inline static const char* packResultToString(PackResult result)
{
	if (result >= PACK_RESULT_COUNT) return "Unknown PACK result";
	return packResultStrings[result];
}