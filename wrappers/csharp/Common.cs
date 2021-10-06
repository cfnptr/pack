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
using System.Runtime.InteropServices;

namespace Pack
{
    public static class Common
    {
        [DllImport("pack")] private static extern void getPackLibraryVersion(
            ref byte majorVersion, ref byte minorVersion, ref byte patchVersion);
        [DllImport("pack")] private static extern PackResult getPackInfo(string filePath, 
            ref byte majorVersion, ref byte minorVersion, ref byte patchVersion, 
            ref bool isLittleEndian, ref ulong itemCount);

        public static void GetPackLibraryVersion(
            ref byte majorVersion, ref byte minorVersion, ref byte patchVersion)
        {
            getPackLibraryVersion(ref majorVersion, ref minorVersion, ref patchVersion);
        }
        public static PackResult GetPackInfo(string filePath, 
            ref byte majorVersion, ref byte minorVersion, ref byte patchVersion, 
            ref bool isLittleEndian, ref ulong itemCount)
        {
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException(nameof(filePath));
            return getPackInfo(filePath, ref majorVersion, ref minorVersion, ref patchVersion, 
                ref isLittleEndian, ref itemCount);
        }
    }
}
