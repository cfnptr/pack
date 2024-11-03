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
 * @brief Pack file reader.
 * @details See the @ref reader.h
 **********************************************************************************************************************/

#pragma once
#include "pack/error.hpp"

#include <thread>
#include <utility>
#include <filesystem>
#include <string_view>

extern "C"
{
#include "pack/reader.h"
}

namespace pack
{

/**
 * @brief Pack reader instance handle.
 * @details See the @ref reader.h
 */
class Reader final
{
private:
	PackReader instance = nullptr;
public:
	/**
	 * @brief Creates a new file pack reader without stream.
	 */
	Reader() = default;

	Reader(const Reader&) = delete;
	Reader(Reader&& r) noexcept : instance(std::exchange(r.instance, nullptr)) { }
	
	Reader& operator=(Reader&) = delete;
	Reader& operator=(Reader&& r) noexcept
	{
		instance = std::exchange(r.instance, nullptr);
		return *this;
	}

	/*******************************************************************************************************************
	 * @brief Creates a new file pack reader instance.
	 * @details See the @ref createFilePackReader().
	 *
	 * @param[in] filePath target Pack file path string
	 * @param isResourcesDirectory read from the resources directory (Android/iOS/macOS only)
	 * @param threadCount max concurrent read thread count
	 * 
	 * @throw Error with a @ref PackResult string on failure.
	 */
	Reader(const filesystem::path& filePath, bool isResourcesDirectory = true,
		uint32_t threadCount = thread::hardware_concurrency())
	{
		auto path = filePath.generic_string();
		auto result = createFilePackReader(path.c_str(), isResourcesDirectory, threadCount, &instance);
		if (result != SUCCESS_PACK_RESULT)
			throw Error(packResultToString(result));
	}

	/**
	 * @brief Destroys pack reader stream.
	 * @details See the @ref destroyPackReader().
	 */
	~Reader() { destroyPackReader(instance); }

	/*******************************************************************************************************************
	 * @brief Opens a new Pack reader stream.
	 * @details See the @ref createFilePackReader().
	 *
	 * @param[in] filePath target Pack file path string
	 * @param isResourcesDirectory read from the resources directory (Android/iOS/macOS only)
	 * @param threadCount max concurrent read thread count
	 * 
	 * @throw Error with a @ref PackResult string on failure.
	 */
	void open(const filesystem::path& filePath, bool isResourcesDirectory = true,
		uint32_t threadCount = thread::hardware_concurrency())
	{
		destroyPackReader(instance);
		auto path = filePath.generic_string();
		auto result = createFilePackReader(path.c_str(), isResourcesDirectory, threadCount, &instance);
		if (result != SUCCESS_PACK_RESULT)
			throw Error(packResultToString(result));
	}

	/**
	 * @brief Closes the current Pack reader stream.
	 * @details See the @ref destroyPackReader().
	 */
	void close() noexcept
	{
		destroyPackReader(instance);
		instance = nullptr;
	}

	/**
	 * @brief Returns true if Pack reader stream is open.
	 * @details See the @ref createFilePackReader().
	 */
	bool isOpen() const noexcept { return instance; }

	/*******************************************************************************************************************
	 * @brief Returns total Pack item count. (MT-Safe)
	 * @details See the @ref getPackItemCount().
	 * @return The total number of the items inside Pack file.
	 */
	uint64_t getItemCount() const noexcept { return getPackItemCount(instance); }

	/**
	 * @brief Returns Pack item index if it is found. (MT-Safe)
	 * @details See the @ref getPackItemIndex().
	 *
	 * @param[in] path item path string used to pack the file
	 * @param[out] index reference to the uint64_t item index
	 * 
	 * @return True if item is found and writes index value, otherwise false.
	 */
	bool getItemIndex(const filesystem::path& path, uint64_t& index) const noexcept
	{
		auto _path = path.generic_string();
		return getPackItemIndex(instance, _path.c_str(), &index);
	}

	/**
	 * @brief Returns Pack item index. (MT-Safe)
	 * @details See the @ref getPackItemIndex().
	 *
	 * @param[in] path item path string used to pack the file
	 * @return The item index in the Pack.
	 * @throw Error if item does not exist.
	 */
	uint64_t getItemIndex(const filesystem::path& path) const
	{
		uint64_t index;
		auto _path = path.generic_string();
		auto result = getPackItemIndex(instance, _path.c_str(), &index);
		if (!result)
			throw Error("Item does not exist");
		return index;
	}

	/*******************************************************************************************************************
	 * @brief Returns Pack item uncompressed data size in bytes. (MT-Safe)
	 * @details See the @ref getPackItemDataSize().
	 *
	 * @param index uint64_t item index
	 * @return The data binary size.
	 */
	uint32_t getItemDataSize(uint64_t index) const noexcept
	{
		return getPackItemDataSize(instance, index);
	}

	/**
	 * @brief Returns Pack item compressed data size in bytes. (MT-Safe)
	 * @details See the @ref getPackItemZipSize().
	 *
	 * @param index uint64_t item index
	 * @return The data binary size, or 0 if item is not compressed.
	 */
	uint32_t getItemZipSize(uint64_t index) const noexcept
	{
		return getPackItemZipSize(instance, index);
	}

	/*******************************************************************************************************************
	 * @brief Reads Pack item binary data. (MT-Safe)
	 * @details See the @ref readPackItemData().
	 *
	 * @param itemIndex uint64_t item index
	 * @param[out] buffer pointer to the buffer where to read item data
	 * @param threadIndex current thread index or 0
	 * 
	 * @throw Error with a @ref PackResult string on failure.
	 */
	void readItemData(uint64_t itemIndex, uint8_t* buffer, uint32_t threadIndex = 0) const
	{
		auto result = readPackItemData(instance, itemIndex, buffer, threadIndex);
		if (result != SUCCESS_PACK_RESULT)
			throw Error(packResultToString(result));
	}

	/**
	 * @brief Reads Pack item binary data. (MT-Safe)
	 * @details See the @ref readPackItemData().
	 *
	 * @param itemIndex uint64_t item index
	 * @param[out] buffer reference to the buffer where to read item data
	 * @param threadIndex current thread index or 0
	 * 
	 * @throw Error with a @ref PackResult string on failure.
	 */
	void readItemData(uint64_t itemIndex, vector<uint8_t>& buffer, uint32_t threadIndex = 0) const
	{
		buffer.resize(getPackItemDataSize(instance, itemIndex));
		auto result = readPackItemData(instance, itemIndex, buffer.data(), threadIndex);
		if (result != SUCCESS_PACK_RESULT)
			throw Error(packResultToString(result));
	}

	/**
	 * @brief Reads Pack item binary data. (MT-Safe)
	 * @details See the @ref readPackItemData().
	 *
	 * @param[in] path item path string used to pack the file
	 * @param[out] buffer reference to the buffer where to read item data
	 * @param threadIndex current thread index or 0
	 * 
	 * @throw Error with a @ref PackResult string on failure.
	 */
	void readItemData(const filesystem::path& path, vector<uint8_t>& buffer, uint32_t threadIndex = 0) const
	{
		auto itemIndex = getItemIndex(path);
		buffer.resize(getPackItemDataSize(instance, itemIndex));
		auto result = readPackItemData(instance, itemIndex, buffer.data(), threadIndex);
		if (result != SUCCESS_PACK_RESULT)
			throw Error(packResultToString(result));
	}

	/*******************************************************************************************************************
	 * @brief Returns Pack item data offset in the archive file. (MT-Safe)
	 * @details See the @ref getPackItemFileOffset().
	 *
	 * @param index uint64_t item index
	 * @return The data offset in the file.
	 */
	uint64_t getItemFileOffset(uint64_t index) const noexcept
	{
		return getPackItemFileOffset(instance, index);
	}

	/**
	 * @brief Returns true if pack item is a reference to a duplicate item. (MT-Safe)
	 * @details See the @ref isPackItemReference().
	 *
	 * @param index uint64_t item index
	 * @return True if item is a reference, otherwise false.
	 */
	bool isItemReference(uint64_t index) const noexcept
	{
		return isPackItemReference(instance, index);
	}

	/**
	 * @brief Returns Pack item path string. (MT-Safe)
	 * @details See the @ref getPackItemPath().
	 *
	 * @param index uint64_t item index
	 * @return The Pack item path sting.
	 */
	string_view getItemPath(uint64_t index) const noexcept
	{
		return getPackItemPath(instance, index);
	}

	/**
	 * @brief Returns Pack ZSTD context array. (MT-Safe)
	 * @details See the @ref getPackZstdContexts().
	 * @return Array of the ZSTD_DCtx* contexts.
	 */
	void** const getZstdContexts() const { return getPackZstdContexts(instance); }

	/*******************************************************************************************************************
	 * @brief Unpacks files from the pack. (MT-Safe)
	 * @details See the @ref unpackFiles().
	 *
	 * @param[in] filePath target Pack file path string
	 * @param printProgress output unpacking progress to the stdout
	 * 
	 * @throw Error with a @ref PackResult string on failure.
	 */
	static void unpack(const filesystem::path& filePath, bool printProgress = false)
	{
		auto path = filePath.generic_string();
		auto result = unpackFiles(path.c_str(), printProgress);
		if (result != SUCCESS_PACK_RESULT)
			throw Error(packResultToString(result));
	}
};

} // pack