// Copyright 2021-2026 Nikita Fediuchin. All rights reserved.
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
 * @brief Pack file reader.
 * 
 * @details
 * Used to read Pack archives at runtime. When opening a file, only information about file names and their locations 
 * in the archive is loaded to save RAM and CPU time. After that, it's possible to selectively extract only the 
 * necessary files from the archive. Loading data from files can be done concurrently from multiple threads.
 */

#pragma once
#include "pack/common.h"

/**
 * @brief Pack reader structure.
 */
typedef struct PackReader_T PackReader_T;
/**
 * @brief Pack reader instance.
 */
typedef PackReader_T* PackReader;

/**
 * @brief Creates a new file pack reader instance.
 * 
 * @details
 * The main function for opening Pack archives. It creates a new Pack reader instance and 
 * reads information about the  location of packed files in the file. Subsequently, 
 * it organizes this information for quick searching and reading data from the archive file. 
 * 
 * @note You should destroy created Pack instance manually.
 *
 * @param[in] filePath target Pack file path string
 * @param dataVersion target packed file data version (0 = ignore data version)
 * @param isResourcesDirectory read from the resources directory (Android/iOS/macOS only)
 * @param threadCount max concurrent read thread count
 * @param[out] packReader pointer to the Pack reader instance
 * 
 * @return The @ref PackResult code and writes reader instance on success.
 * 
 * @retval SUCCESS_PACK_RESULT on success
 * @retval FAILED_TO_ALLOCATE_PACK_RESULT if out of memory
 * @retval FAILED_TO_GET_DIRECTORY_PACK_RESULT if failed to get resources directory path
 * @retval FAILED_TO_OPEN_FILE_PACK_RESULT if file doesn't exist
 * @retval FAILED_TO_CREATE_ZSTD_PACK_RESULT if failed to create ZSTD contexts
 * @retval FAILED_TO_READ_FILE_PACK_RESULT if failed to read Pack file data
 * @retval FAILED_TO_SEEK_FILE_PACK_RESULT if failed to seek Pack file data
 * @retval BAD_FILE_TYPE_PACK_RESULT if file is not a Pack archive
 * @retval BAD_FILE_VERSION_PACK_RESULT if different Pack file version
 * @retval BAD_FILE_ENDIANNESS_PACK_RESULT if different Pack file data endianness
 * @retval BAD_DATA_SIZE_PACK_RESULT if bad Pack file data size
 * @retval BAD_FILE_DATA_VERSION_PACK_RESULT if bad packed file data version
 */
PackResult createFilePackReader(const char* filePath, uint32_t dataVersion,
	bool isResourcesDirectory, uint32_t threadCount, PackReader* packReader);

/**
 * @brief Destroys Pack reader instance.
 * @param packReader pack reader instance or NULL
 */
void destroyPackReader(PackReader packReader);

/**
 * @brief Returns total Pack item count. (MT-Safe)
 * @details You can use the returned value to iterate over all Pack archive files.
 * 
 * @param packReader pack reader instance
 * 
 * @return The total number of the items inside Pack file.
 * @retval Integer between 1 and 18,446,744,073,709,551,615
 */
uint64_t getPackItemCount(PackReader packReader);

/**
 * @brief Returns Pack item index if it is found. (MT-Safe)
 * @details You can use the returned item index to obtain it's complete information.
 *
 * @param packReader pack reader instance
 * @param[in] path item path string used to pack the file
 * @param[out] index pointer to the uint64_t item index
 * 
 * @return True if item is found and writes index value, otherwise false.
 * @retval true if the item has been found
 * @retval false if file doesn't exist in the Pack
 */
bool getPackItemIndex(PackReader packReader, const char* path, uint64_t* index);

/***********************************************************************************************************************
 * @brief Returns Pack item uncompressed data size in bytes. (MT-Safe)
 * @details Use the returned item binary size to allocate a memory block for data reading.
 *
 * @param packReader pack reader instance
 * @param index uint64_t item index
 * 
 * @return The data binary size.
 * @retval Integer between 1 and 4,294,967,295 (4GB)
 */
uint32_t getPackItemDataSize(PackReader packReader, uint64_t index);

/**
 * @brief Returns Pack item compressed data size in bytes. (MT-Safe)
 * @details You can use the returned item zipped binary size for statistics or to determine if it is compressed.
 *
 * @param packReader pack reader instance
 * @param index uint64_t item index
 * 
 * @return The data binary size, or 0 if item is not compressed.
 * @retval Integer between 1 and 4,294,967,295 if compressed. (4GB)
 * @retval 0 if item is not compressed
 */
uint32_t getPackItemZipSize(PackReader packReader, uint64_t index);

/**
 * @brief Reads Pack item binary data. (MT-Safe)
 * 
 * @details
 * The main function for loading item data from a file into memory. Additionally, 
 * we can pass the index of the current thread and load the data asynchronously.
 *
 * @param packReader pack reader instance
 * @param itemIndex uint64_t item index
 * @param[out] buffer target buffer where to read the item data
 * @param threadIndex current thread index or 0
 * 
 * @return The @ref PackResult code and writes reader instance on success.
 * 
 * @retval SUCCESS_PACK_RESULT - successful operation
 * @retval FAILED_TO_ALLOCATE_PACK_RESULT - out of memory
 * @retval FAILED_TO_READ_FILE_PACK_RESULT - failed to read Pack file data
 * @retval FAILED_TO_SEEK_FILE_PACK_RESULT - failed to seek Pack file data
 * @retval FAILED_TO_DECOMPRESS_PACK_RESULT - compressed file data is damaged
 */
PackResult readPackItemData(PackReader packReader, uint64_t itemIndex, uint8_t* buffer, uint32_t threadIndex);

/***********************************************************************************************************************
 * @brief Returns Pack item data offset in the archive file. (MT-Safe)
 * @details Internally used to read an item data from the archive file.
 *
 * @param packReader pack reader instance
 * @param index uint64_t item index
 * 
 * @return The data offset in the file.
 */
uint64_t getPackItemFileOffset(PackReader packReader, uint64_t index);

/**
 * @brief Returns true if pack item is a reference to a duplicate item. (MT-Safe)
 * @details Internally Pack shares binary data block between the same items.
 *
 * @param packReader pack reader instance
 * @param index uint64_t item index
 * 
 * @return True if item is a reference, otherwise false.
 */
bool isPackItemReference(PackReader packReader, uint64_t index);

/**
 * @brief Returns Pack item path string. (MT-Safe)
 * @details You can get the Pack item path when iterating over all items.
 * @warning You should not free the returned string.
 *
 * @param packReader pack reader instance
 * @param index uint64_t item index
 * 
 * @return The Pack item path sting.
 */
const char* getPackItemPath(PackReader packReader, uint64_t index);

/**
 * @brief Returns true if data was compressed with fast-read algorithm. (MT-Safe)
 * @param packReader pack reader instance
 */
bool isPackPreferSpeed(PackReader packReader);

/**
 * @brief Returns Pack ZSTD context array. (MT-Safe)
 * @details Can be used to share the ZSTD contexts in the program.
 * @param packReader pack reader instance
 * @return Array of the ZSTD_DCtx* contexts.
 */
void** const getPackZstdContexts(PackReader packReader);
/**
 * @brief Returns Pack concurrent read thread count. (MT-Safe)
 * @param packReader pack reader instance
 */
uint32_t getPackThreadCount(PackReader packReader);

/**
 * @brief Reduces internal Pack reader memory consumption.
 * @param packReader pack reader instance
 */
void shrinkPack(PackReader packReader);

/***********************************************************************************************************************
 * @brief Unpacks files from the pack. (MT-Safe)
 * @details This function is useful when we need to create a unpacker for debugging a program.
 *
 * @param[in] filePath target Pack file path string
 * @param printProgress output unpacking progress to the stdout
 * 
 * @return The @ref PackResult code and unpacks files on success.
 *
 * @retval SUCCESS_PACK_RESULT on success
 * @retval FAILED_TO_ALLOCATE_PACK_RESULT if out of memory
 * @retval FAILED_TO_GET_DIRECTORY_PACK_RESULT if failed to get resources directory path
 * @retval FAILED_TO_OPEN_FILE_PACK_RESULT if file doesn't exist
 * @retval FAILED_TO_CREATE_ZSTD_PACK_RESULT if failed to create ZSTD contexts
 * @retval FAILED_TO_READ_FILE_PACK_RESULT if failed to read Pack file data
 * @retval FAILED_TO_SEEK_FILE_PACK_RESULT if failed to seek Pack file data
 * @retval BAD_FILE_TYPE_PACK_RESULT if file is not a Pack archive
 * @retval BAD_FILE_VERSION_PACK_RESULT if different Pack file version
 * @retval BAD_FILE_ENDIANNESS_PACK_RESULT if different Pack file data endianness
 * @retval BAD_DATA_SIZE_PACK_RESULT if bad Pack file data size
 * @retval BAD_FILE_DATA_VERSION_PACK_RESULT if bad packed file data version
 */
PackResult unpackFiles(const char* filePath, bool printProgress);