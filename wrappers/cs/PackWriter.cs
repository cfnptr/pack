using System;
using System.Runtime.InteropServices;

namespace Pack
{
    public static class PackWriter
    {
        [DllImport(Common.LibraryPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)] private static extern PackResult packFiles(string packPath, ulong fileCount, string[] filePaths, float zipThreshold, bool printProgress, OnPackFile callback);

        public delegate void OnPackFile(UInt64 index, object o);
        public static PackResult PackFiles(in string packPath, in string[] filePaths, in float zipTreshold, in bool printProgress, OnPackFile callback)
        {
            if (string.IsNullOrEmpty(packPath))
                throw new ArgumentNullException(nameof(packPath));
            if (filePaths.Length == 0)
                throw new ArgumentNullException(nameof(filePaths));

            var res = packFiles(packPath, (ulong)filePaths.Length / 2, filePaths, zipTreshold, printProgress, callback, ret);
            return res;
        }
    }
}
