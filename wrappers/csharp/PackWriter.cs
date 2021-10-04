using System;
using System.Runtime.InteropServices;

namespace Pack
{
    public static class PackWriter
    {
#if PACK_TARGET_LINUX
        private const string LibraryPath = "libpack.so";
#elif PACK_TARGET_MACOS
        private const string LibraryPath = "libpack.dylib";
#elif PACK_TARGET_WINDOWS
        private const string LibraryPath = "pack.dll";
#else
#error Unspecified target operating system
#endif
        
        [DllImport(LibraryPath)] private static extern PackResult packFiles(string packPath, ulong fileCount, string[] filePaths, bool printProgress);

        public static PackResult PackFiles(string packPath, string[] filePaths, bool printProgress)
        {
            if (string.IsNullOrEmpty(packPath))
                throw new ArgumentNullException(nameof(packPath));
            if (filePaths.Length == 0)
                throw new ArgumentNullException(nameof(filePaths));
            return packFiles(packPath, (ulong)filePaths.Length, filePaths, printProgress);
        }
    }
}
