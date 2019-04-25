#ifndef FILEIO_HEADER
#define FILEIO_HEADER
    
    unsigned char* readIntoMemory(char* filename);
    static long length;
    long getLength();
    char* getCompressedName(char* filename);
    char* getDecompressedName(char* filename);
#endif