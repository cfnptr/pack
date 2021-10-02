using System;
using System.Runtime.InteropServices;

namespace Pack
{
    public static class PackWriter
    {
        private const string LibraryPath = "pack.dll";
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
