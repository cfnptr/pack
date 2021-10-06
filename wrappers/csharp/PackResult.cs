// Copyright 2021 Nikita Fediuchin. All rights reserved.
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

using System;

namespace Pack
{
    public enum PackResult
    {
        Success = 0,
        FailedToAllocate = 1,
        FailedToCreateZSTD = 2,
        FailedToCreateFile = 3,
        FailedToOpenFile = 4,
        FailedToWriteFile = 5,
        FailedToReadFile = 6,
        FailedToSeekFile = 7,
        FailedToDecompress = 8,
        FailedToGetItem = 9,
        BadDataSize = 10,
        BadFileType = 11,
        BadFileVersion = 12,
        BadFileEndianness = 13,
        Count = 14,
    }
    
    public class PackException : Exception
    {
        public PackException(PackResult result) :
            base(result.ToString()) { }
        public PackException(string message) :
            base(message) { }
    }
}
