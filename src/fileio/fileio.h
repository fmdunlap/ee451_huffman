#ifndef FILEIO_HEADER
#define FILEIO_HEADER
    
    char* readIntoMemory(char* filename);
    static long length;
    long getLength();
    char* getCompressedName(char* filename);
#endif