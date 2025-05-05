using System;
using System.Runtime.InteropServices;

namespace Pack
{
    public class PackReader
    {
        const CallingConvention conversion = CallingConvention.Cdecl;
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern byte createFilePackReader(string filePath, bool isResourcesDirectory, uint threadCount, ref IntPtr packReader);
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern void destroyPackReader(IntPtr packReader);
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern ulong getPackItemCount(IntPtr packReader);
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern bool getPackItemIndex(IntPtr packReader, string path, ref ulong index);
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern uint getPackItemDataSize(IntPtr packReader, ulong index);
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern uint getPackItemZipSize(IntPtr packReader, ulong index);
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern ulong getPackItemFileOffset(IntPtr packReader, ulong index);
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern bool isPackItemReference(IntPtr packReader, ulong index);

        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern IntPtr getPackItemPath(IntPtr packReader, ulong index);
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern PackResult readPackItemData(IntPtr packReader, ulong index, byte[] data, uint threadIndex);
        [DllImport(Common.LibraryPath, CallingConvention = conversion)] protected static extern PackResult unpackFiles(string filePath, bool printProgress);

        protected IntPtr Handle;

        public ulong ItemCount => getPackItemCount(Handle);

        public PackReader(string filePath, bool isResourceDirectory = false, uint threadCount = 1)
        {
            var handle = IntPtr.Zero;
            var result = createFilePackReader(filePath, isResourceDirectory, threadCount, ref handle);

            if (result != 0)
                throw new Exception(result.ToString());

            Handle = handle;
        }
        ~PackReader()
        {
            destroyPackReader(Handle);
        }

        public bool GetItemIndex(in string path, ref ulong index)
        {
            if (string.IsNullOrEmpty(path))
                throw new ArgumentNullException(nameof(path));
            if (path.Length > byte.MaxValue)
                throw new ArgumentOutOfRangeException(nameof(path));
            return getPackItemIndex(Handle, path, ref index);
        }
        public uint GetItemDataSize(in ulong index)
        {
            if (index >= ItemCount)
                throw new ArgumentOutOfRangeException(nameof(index));
            return getPackItemDataSize(Handle, index);
        }
        public string GetItemPath(in ulong index)
        {
            if (index >= ItemCount)
                throw new ArgumentOutOfRangeException(nameof(index));

            var path = getPackItemPath(Handle, index);
            return Marshal.PtrToStringAnsi(path);
        }

        public PackResult ReadItemData(in string path, ref byte[] data, in uint thread = 0)
        {
            if (string.IsNullOrEmpty(path))
                throw new ArgumentNullException(nameof(path));
            if (path.Length > byte.MaxValue)
                throw new ArgumentOutOfRangeException(nameof(path));

            ulong index = 0;
            if (getPackItemIndex(Handle, path, ref index))
            {
                return ReadItemData(index, ref data, thread);
            }
            return PackResult.FailedToGetItem;

        }

        public PackResult ReadItemData(in ulong index, ref byte[] data, in uint thread = 0)
        {
            var result = readPackItemData(Handle, index, data, thread);
            if (result != PackResult.Success)
                return result;
            return PackResult.Success;
        }

        public byte[]? ReadItemData(in ulong index, in uint thread = 0)
        {
            uint size = getPackItemDataSize(Handle, index);
            byte[] data = new byte[size];
            ReadItemData(index, ref data, thread);
            return data;
        }
        public byte[]? ReadItemData(in string path, in uint thread = 0)
        {
            ulong index = 0;
            if (getPackItemIndex(Handle, path, ref index))
            {
                uint size = getPackItemDataSize(Handle, index);
                byte[] data = new byte[size];
                ReadItemData(index, ref data, thread);
                return data;
            }
            return null;

        }
        public static PackResult UnpackFiles(in string filePath, in bool printProgress)
        {
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException(nameof(filePath));
            return unpackFiles(filePath, printProgress);
        }
    }
}
