using System.Runtime.InteropServices;

namespace Pack
{
    [StructLayout(LayoutKind.Sequential)]
    public struct PackItemHeader
    {
        public uint zipState;
        public uint dataSize;
        public byte pathSize;
        public byte isReference;
        public ulong dataOffset;
    }
}
