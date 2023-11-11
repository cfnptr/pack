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
#include <thread>
#include <utility>
#include <exception>
#include <filesystem>
#include <string_view>

extern "C"
{
#include "pack/reader.h"
}

namespace pack
{

using namespace std;

/*
 * Pack reader instance handle.
 */
class Reader final
{
private:
	PackReader instance = nullptr;
public:
	/*
	 * Create a new file pack reader.
	 */
	Reader() = default;

	Reader(const Reader&) = delete;
	Reader(Reader&& r) noexcept : instance(
		std::exchange(r.instance, nullptr)) { }
	
	Reader& operator=(Reader&) = delete;
	Reader& operator=(Reader&& r) noexcept
	{
		instance = std::exchange(r.instance, nullptr);
		return *this;
	}

	/*
	 * Create a new file pack reader and open stream.
	 * Throws runtime exception on failure.
	 *
	 * isResourcesDirectory - read from resources directory. (macOS)
	 * threadCount - concurrent access thread count.
	 */
	Reader(const filesystem::path& filePath, bool isResourcesDirectory = true,
		uint32_t threadCount = thread::hardware_concurrency())
	{
		auto path = filePath.generic_string();
		auto result = createFilePackReader(path.c_str(),
			isResourcesDirectory, threadCount, &instance);
		if (result != SUCCESS_PACK_RESULT)
		{
			throw runtime_error(packResultToString(result) +
				(", path: " + filePath.generic_string()));
		}
	}
	/*
	 * Destroys pack reader stream.
	 */
	~Reader() { destroyPackReader(instance); }

	/*
	 * Open a new pack reader stream.
	 * Throws runtime exception on failure.
	 *
	 * filePath - file path string.
	 * isResourcesDirectory - read from resources directory. (macOS)
	 * threadCount - concurrent access thread count.
	 */
	void open(const filesystem::path& filePath, bool isResourcesDirectory = true,
		uint32_t threadCount = thread::hardware_concurrency())
	{
		destroyPackReader(instance);
		auto path = filePath.generic_string();
		auto result = createFilePackReader(path.c_str(),
			isResourcesDirectory, threadCount, &instance);
		if (result != SUCCESS_PACK_RESULT)
		{
			throw runtime_error(packResultToString(result) +
				(", path: " + filePath.generic_string()));
		}
	}
	/*
	 * Close current pack reader stream.
	 */
	void close() noexcept
	{
		destroyPackReader(instance);
		instance = nullptr;
	}
	/*
	 * Returns true if reader stream is open.
	 */
	bool isOpen() const noexcept { return instance; }

	/*
	 * Returns pack reader item count. (MT-Safe)
	 */
	uint64_t getItemCount() const noexcept { return getPackItemCount(instance); }

	/*
	 * Search for the pack item index. (MT-Safe)
 	 * Returns true if item exists.
	 *
	 * path - item path string.
	 * index - reference to the pack item index.
	 */
	bool getItemIndex(const filesystem::path& path, uint64_t &index) const noexcept
	{
		auto _path = path.generic_string();
		return getPackItemIndex(instance, _path.c_str(), &index);
	}
	/*
	 * Get pack item index. (MT-Safe)
 	 * Throws runtime exception on failure.
	 *
	 * path - item path string.
	 */
	uint64_t getItemIndex(const filesystem::path& path) const
	{
		uint64_t index;
		auto _path = path.generic_string();
		auto result = getPackItemIndex(instance, _path.c_str(), &index);
		if (!result) throw runtime_error("Item is not exist");
		return index;
	}

	/*
	 * Returns pack item data size. (MT-Safe)
	 * index - item index.
	 */
	uint32_t getItemDataSize(uint64_t index) const noexcept
	{
		return getPackItemDataSize(instance, index);
	}
	/*
	 * Returns pack item zip size, or 0 if uncompressed. (MT-Safe)
	 * index - item index.
	 */
	uint32_t getItemZipSize(uint64_t index) const noexcept
	{
		return getPackItemZipSize(instance, index);
	}

	/*
	 * Returns pack item data offset in the file. (MT-Safe)
	 * index - item index.
	 */
	uint64_t getItemFileOffset(uint64_t index) const noexcept
	{
		return getPackItemFileOffset(instance, index);
	}
	/*
	 * Returns true if pack item is a reference to duplicate item. (MT-Safe)
	 * index - item index.
	 */
	bool isItemReference(uint64_t index) const noexcept
	{
		return isPackItemReference(instance, index);
	}

	/*
	 * Returns pack item path string. (MT-Safe)
	 * index - item index.
	 */
	string_view getItemPath(uint64_t index) const noexcept
	{
		return getPackItemPath(instance, index);
	}

	/*
	 * Read pack item data. (MT-Safe)
	 * Throws runtime exception on failure.
	 *
	 * itemIndex - item index.
	 * buffer - item data buffer.
	 * threadIndex - current thread index.
	 */
	void readItemData(uint64_t itemIndex,
		uint8_t* buffer, uint32_t threadIndex = 0) const
	{
		auto result = readPackItemData(instance, itemIndex, buffer, threadIndex);
		if (result != SUCCESS_PACK_RESULT)
		{
			throw runtime_error(packResultToString(result) +
				(", index: " + to_string(itemIndex)));
		}
	}
	/*
	 * Read pack item data. (MT-Safe)
	 * Throws runtime exception on failure.
	 *
	 * itemIndex - item index.
	 * buffer - item data buffer.
	 * threadIndex - current thread index.
	 */
	void readItemData(uint64_t itemIndex,
		vector<uint8_t>& buffer, uint32_t threadIndex = 0) const
	{
		buffer.resize(getPackItemDataSize(instance, itemIndex));
		auto result = readPackItemData(instance, itemIndex, buffer.data(), threadIndex);
		if (result != SUCCESS_PACK_RESULT)
		{
			throw runtime_error(packResultToString(result) +
				(", index: " + to_string(itemIndex)));
		}
	}
	/*
	 * Read pack item data. (MT-Safe)
	 * Throws runtime exception on failure.
	 *
	 * itemIndex - item index.
	 * buffer - item data buffer.
	 * threadIndex - current thread index.
	 */
	void readItemData(const filesystem::path& path,
		vector<uint8_t>& buffer, uint32_t threadIndex = 0) const
	{
		auto itemIndex = getItemIndex(path);
		buffer.resize(getPackItemDataSize(instance, itemIndex));
		
		auto result = readPackItemData(instance, itemIndex, buffer.data(), threadIndex);
		if (result != SUCCESS_PACK_RESULT)
		{
			throw runtime_error(packResultToString(result) +
				(", path: " + path.generic_string()));
		}
	}

	/*
	 * Returns pack ZSTD contexts. (MT-Safe)
	 * packReader - pack reader instance.
	 */
	void** const getZstdContexts() const { return getPackZstdContexts(instance); }

	/*
	 * Unpack files from the pack. (MT-Safe)
	 * Throws runtime exception on failure.
	 *
	 * filePath - file path string.
	 * printProgress - printf reading progress.
	 */
	static void unpack(const filesystem::path& filePath, bool printProgress = false)
	{
		auto path = filePath.generic_string();
		auto result = unpackFiles(path.c_str(), printProgress);
		if (result != SUCCESS_PACK_RESULT)
		{
			throw runtime_error(packResultToString(result) +
				(", path: " + filePath.generic_string()));
		}
	}
};

} // pack