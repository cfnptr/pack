namespace Pack
{
    public enum PackResult
    {
        Success = 0,
        FailedToAllocate = 1,
        FailedToCreateZSTD = 2,
        FailedToCreateFile = 3,
        FailedToOpenFile = 4,
        FailedToWriteFile = 5,
        FailedToReadFile = 6,
        FailedToSeekFile = 7,
        FailedToGetDirectory = 8,
        FailedToDecompress = 9,
        FailedToGetItem = 10,
        BadDataSize = 11,
        BadFileType = 12,
        BadFileVersion = 13,
        BadFileEndianness = 14,
        PackResultCount = 15
    }
}
