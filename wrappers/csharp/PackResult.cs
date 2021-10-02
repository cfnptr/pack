namespace Pack
{
    public enum PackResult
    {
        Success = 0,
        FailedToAllocate = 1,
        FailedToCreateZSTD = 2,
        FailedToOpenFile = 3,
        FailedToWriteFile = 4,
        FailedToReadFile = 5,
        FailedToSeekFile = 6,
        FailedToDecompress = 7,
        BadDataSize = 8,
        BadFileType = 9,
        BadFileVersion = 10,
        BadFileEndianness = 11,
    }
}
