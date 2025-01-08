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
 * @brief Pack file writer.
 * 
 * @details
 * Used to pack files into a Pack archives. During packing, files are compressed with maximum ZSTD compression, or 
 * written as they are if they are already compressed. Additionally, each file is checked for data duplication, 
 * and in cases where two files are identical, a reference to the data of the already packed file is written.
 **********************************************************************************************************************/

#pragma once
#include "pack/common.h"

/**
 * @brief File packing callback.
 * 
 * @param itemIndex current packing item index
 * @param argument callback agument, or NULL
 */
typedef void(*OnPackFile)(uint64_t itemIndex, void* argument);

/**
 * @brief Packs files to the Pack archive.
 * 
 * @details
 * The main function for packing a Pack archives. It reads the provided files and compresses them with maximum 
 * possible compression. You can speed up the runtime file decompression by specifying a zipThreshold value, as if 
 * after compression, we achieve only 10% compression, then decompression will consume more resources than we 
 * save on file size. The optimal float value for the zipThreshold is 0.1f.
 *
 * @param[in] packPath output Pack file path string
 * @param fileCount file count to pack
 * @param[in] fileItemPaths pack file and item path string array (file/item, file/item...)
 * @param zipThreshold compression threshold (0.0 - 1.0 range)
 * @param printProgress output packing progress to the stdout
 * @param[in] onPackFile file packing callback, or NULL
 * @param[in] argument file packing callback argument, or NULL
 * 
 * @return The @ref PackResult code.
 */
PackResult packFiles(const char* packPath, uint64_t fileCount, const char** fileItemPaths,
	float zipThreshold, bool printProgress, OnPackFile onPackFile, void* argument);