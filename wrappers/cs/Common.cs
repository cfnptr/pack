using System;
using System.Runtime.InteropServices;

namespace Pack
{
    public static class Common
    {
        public const string LibraryPath = "pack";
        [DllImport(LibraryPath)] private static extern void getPackLibraryVersion(ref byte majorVersion, ref byte minorVersion, ref byte patchVersion);
        [DllImport(LibraryPath)] private static extern byte readPackHeader(string filePath, ref PackHeader header);

        public static void GetPackLibraryVersion(ref byte majorVersion, ref byte minorVersion, ref byte patchVersion)
        {
            getPackLibraryVersion(ref majorVersion, ref minorVersion, ref patchVersion);
        }
        public static PackResult GetPackInfo(in string filePath, out PackHeader header)
        {

            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException(nameof(filePath));
            header = new();
            return (PackResult)readPackHeader(filePath, ref header);
        }
    }
}
