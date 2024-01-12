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

        public bool GetItemIndex(string path, ref ulong index)
        {
            if (string.IsNullOrEmpty(path))
                throw new ArgumentNullException(nameof(path));
            if (path.Length > byte.MaxValue)
                throw new ArgumentOutOfRangeException(nameof(path));
            return getPackItemIndex(Handle, path, ref index);
        }
        public uint GetItemDataSize(ulong index)
        {
            if (index >= ItemCount)
                throw new ArgumentOutOfRangeException(nameof(index));
            return getPackItemDataSize(Handle, index);
        }
        public string GetItemPath(ulong index)
        {
            if (index >= ItemCount)
                throw new ArgumentOutOfRangeException(nameof(index));

            var path = getPackItemPath(Handle, index);
            return Marshal.PtrToStringAnsi(path);
        }

        public PackResult ReadItemData(string path, ref byte[] data, uint thread = 0)
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

        public PackResult ReadItemData(ulong index, ref byte[] data, uint thread = 0)
        {
            var result = readPackItemData(Handle, index, data, thread);
            if (result != PackResult.Success)
                return result;
            return PackResult.Success;
        }

        public byte[]? ReadItemData(ulong index, uint thread = 0)
        {
            uint size = getPackItemDataSize(Handle, index);
            byte[] data = new byte[size];
            ReadItemData(index, ref data, thread);
            return data;
        }
        public byte[]? ReadItemData(string path, uint thread = 0)
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
        /*
        public PackResult ReadItemData(string path, ref byte[] data)
        {
            var buffer = IntPtr.Zero;
            uint size = (uint) data.Length;

            var result = ReadItemData(path, ref buffer, ref size);

            if (result != PackResult.Success)
                return result;

            if (size > int.MaxValue)
                throw new ApplicationException("Unsupported item data size");

            data = new byte[size];
            Marshal.Copy(buffer, data, 0, (int)size);
            return PackResult.Success;
        }

        public PackResult ReadItemData(ulong index, ref byte[] data)
        {
            var buffer = IntPtr.Zero;
            uint size = (uint)data.Length;

            var result = ReadItemData(index, ref buffer);

            if (result != PackResult.Success)
                return result;

            if (size > int.MaxValue)
                throw new ApplicationException("Unsupported item data size");

            data = new byte[size];
            Marshal.Copy(buffer, data, 0, (int)size);
            return PackResult.Success;
        }

        public PackResult ReadItemData(ulong index, ref string data)
        {
            byte[] buffer = null;
            var result = ReadItemData(index, ref buffer);

            if (result != PackResult.Success)
                return result;

            data = Encoding.UTF8.GetString(buffer);
            return PackResult.Success;
        }
        public PackResult ReadItemData(string path, ref string data)
        {
            byte[] buffer = null;
            var result = ReadItemData(path, ref buffer);

            if (result != PackResult.Success)
                return result;

            data = Encoding.UTF8.GetString(buffer);
            return PackResult.Success;
        }
        */
        /*
        public void FreeBuffers()
        {
            freePackReaderBuffers(Handle);
        }
        */
        public static PackResult UnpackFiles(string filePath, bool printProgress)
        {
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException(nameof(filePath));
            return unpackFiles(filePath, printProgress);
        }
    }
}
