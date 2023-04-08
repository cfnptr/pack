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
	 * Create a new file pack reader instance.
	 * Throws runtime exception on failure.
	 *
	 * filePath - file path string.
	 * dataBufferCapacity - initial data buffer capacity or 0.
	 * isResourcesDirectory - read from resources directory. (macOS)
	 */
	Reader(string_view filePath, uint32_t dataBufferCapacity = 0,
		bool isResourcesDirectory = false)
	{
		auto result = createFilePackReader(filePath.data(),
			dataBufferCapacity, isResourcesDirectory, &instance);
		if (result != SUCCESS_PACK_RESULT)
			throw runtime_error(packResultToString(result));
	}
	/*
	 * Destroys pack reader instance.
	 */
	~Reader() { destroyPackReader(instance); }
	
	/*
	 * Returns pack reader item count.
	 */
	uint64_t getItemCount() const noexcept { return getPackItemCount(instance); }

	/*
	 * Search for the pack item index.
 	 * Returns true if item exists.
	 *
	 * path - item path string.
	 * index - reference to the pack item index.
	 */
	bool getItemIndex(string_view path, uint64_t &index) const noexcept
	{
		return getPackItemIndex(instance, path.data(), &index);
	}
	/*
	 * Returns pack item data size.
	 * index - item index.
	 */
	uint32_t getItemDataSize(uint64_t index) const noexcept
	{
		return getPackItemDataSize(instance, index);
	}
	/*
	 * Returns pack item zip size, or 0 if uncompressed.
	 * index - item index.
	 */
	uint32_t getItemZipSize(uint64_t index) const noexcept
	{
		return getPackItemZipSize(instance, index);
	}

	/*
	 * Returns pack item data offset in the file.
	 * index - item index.
	 */
	uint64_t getItemFileOffset(uint64_t index) const noexcept
	{
		return getPackItemFileOffset(instance, index);
	}
	/*
	 * Returns true if pack item is a reference to duplicate item.
	 * index - item index.
	 */
	bool isItemReference(uint64_t index) const noexcept
	{
		return isPackItemReference(instance, index);
	}

	/*
	 * Returns pack item path string.
	 * index - item index.
	 */
	string_view getItemPath(uint64_t index) const noexcept
	{
		return getPackItemPath(instance, index);
	}

	/*
	 * Read pack item data.
	 * Throws runtime exception on failure.
	 *
	 * index - item index.
	 * data - reference to the item data buffer.
	 * size - reference to the item data size.
	 */
	void readItemData(uint64_t index,
		const uint8_t*& data, uint32_t& size) const
	{
		auto result = readPackItemData(instance, index, &data, &size);
		if (result != SUCCESS_PACK_RESULT)
			throw runtime_error(packResultToString(result));
	}
	/*
	 * Read pack item data.
	 * Throws runtime exception on failure.
	 *
	 * path - item path string.
	 * data - reference to the item data buffer.
	 * size - reference to the item data size.
	 */
	void readItemData(string_view path,
		const uint8_t*& data, uint32_t& size) const
	{
		auto result = readPackPathItemData(instance, path.data(), &data, &size);
		if (result != SUCCESS_PACK_RESULT)
			throw runtime_error(packResultToString(result));
	}
	
	/*
	 * Free pack reader buffers. (Decreases reader memory usage)
	 */
	void freeReaderBuffers(PackReader packReader) const noexcept
	{
		freePackReaderBuffers(instance);
	}

	/*
	 * Unpack files from the pack.
	 * Throws runtime exception on failure.
	 *
	 * filePath - file path string.
	 * printProgress - printf reading progress.
	 */
	static void unpack(string_view filePath, bool printProgress = false)
	{
		auto result = unpackFiles(filePath.data(), printProgress);
		if (result != SUCCESS_PACK_RESULT)
			throw runtime_error(packResultToString(result));
	}
};

} // pack