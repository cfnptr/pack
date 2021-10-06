using System;
using System.Text;
using System.Runtime.InteropServices;

namespace Pack
{
    public class PackReader
    {
        [DllImport("pack")] private static extern PackResult createPackReader(
            string filePath, ref IntPtr packReader);
        [DllImport("pack")] private static extern void destroyPackReader(IntPtr packReader);
        [DllImport("pack")] private static extern ulong getPackItemCount(IntPtr packReader);
        [DllImport("pack")] private static extern bool getPackItemIndex(
            IntPtr packReader, string path, ref ulong index);
        [DllImport("pack")] private static extern uint getPackItemDataSize(
            IntPtr packReader, ulong index);
        [DllImport("pack")] private static extern string getPackItemPath(
            IntPtr packReader, ulong index);
        [DllImport("pack")] private static extern PackResult readPackItemData(
            IntPtr packReader, ulong index, ref IntPtr data, ref uint size);
        [DllImport("pack")] private static extern PackResult readPackPathItemData(
            IntPtr packReader, string path, ref IntPtr data, ref uint size);
        [DllImport("pack")] private static extern void freePackReaderBuffers(IntPtr packReader);
        [DllImport("pack")] private static extern PackResult unpackFiles(
            string filePath, ref ulong fileCount, bool printProgress);

        private readonly IntPtr _handle;
        public IntPtr Handle => _handle;

        public ulong ItemCount => getPackItemCount(Handle);

        public PackReader(string filePath)
        {
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException(nameof(filePath));

            var result = createPackReader(filePath, ref _handle);

            if (result != PackResult.Success)
                throw new PackException(result);
        }
        ~PackReader()
        {
            destroyPackReader(_handle);
        }

        public bool GetItemIndex(string path, ref ulong index)
        {
            if (string.IsNullOrEmpty(path))
                throw new ArgumentNullException(nameof(path));
            if (path.Length > byte.MaxValue)
                throw new ArgumentOutOfRangeException(nameof(path));
            return getPackItemIndex(_handle, path, ref index);
        }
        public uint GetItemDataSize(ulong index)
        {
            if (index >= ItemCount)
                throw new ArgumentOutOfRangeException(nameof(index));
            return getPackItemDataSize(_handle, index);
        }
        public string GetItemPath(ulong index)
        {
            if (index >= ItemCount)
                throw new ArgumentOutOfRangeException(nameof(index));
            return getPackItemPath(_handle, index);
        }
        
        public PackResult ReadItemData(ulong index, ref IntPtr data, ref uint size)
        {
            if (index >= ItemCount)
                throw new ArgumentOutOfRangeException(nameof(index));
            return readPackItemData(_handle, index, ref data, ref size);
        }
        public PackResult ReadItemData(string path, ref IntPtr data, ref uint size)
        {
            if (string.IsNullOrEmpty(path))
                throw new ArgumentNullException(nameof(path));
            if (path.Length > byte.MaxValue)
                throw new ArgumentOutOfRangeException(nameof(path));
            return readPackPathItemData(_handle, path, ref data, ref size);
        }

        public PackResult ReadItemData(ulong index, ref byte[] data)
        {
            var buffer = IntPtr.Zero;
            var size = uint.MinValue;

            var result = ReadItemData(index, ref buffer, ref size);

            if (result != PackResult.Success)
                return result;

            if (size > int.MaxValue)
                throw new PackException("Unsupported item data size");
            
            data = new byte[size];
            Marshal.Copy(buffer, data, 0, (int)size);
            return PackResult.Success;
        }
        public PackResult ReadItemData(string path, ref byte[] data)
        {
            var buffer = IntPtr.Zero;
            var size = uint.MinValue;

            var result = ReadItemData(path, ref buffer, ref size);

            if (result != PackResult.Success)
                return result;

            if (size > int.MaxValue)
                throw new PackException("Unsupported item data size");

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

        public void FreeBuffers()
        {
            freePackReaderBuffers(_handle);
        }

        public static PackResult UnpackFiles(string filePath, ref ulong fileCount, bool printProgress)
        {
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException(nameof(filePath));
            return unpackFiles(filePath, ref fileCount, printProgress);
        }
    }
}
