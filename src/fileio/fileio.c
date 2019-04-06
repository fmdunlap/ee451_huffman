#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>

char* readIntoMemory(char* filename){
    FILE * fileToCompress;
    unsigned char *fileBuffer;

    fileToCompress = fopen(filename, "rb");

    // We can get the length of the file by jumping to the end, getting the 
    // byte offset, and then rewinding to the beginning.
    fseek(fileToCompress, 0, SEEK_END);
    length = ftell(fileToCompress);
    printf("File is of length: %i\n", length);
    rewind(fileToCompress);

    fileBuffer = (unsigned char*)malloc((length+1)*sizeof(unsigned char));
    length = fread(fileBuffer, sizeof(unsigned char), length, fileToCompress);
    printf("Read %d bytes.\n", length);
    fclose(fileToCompress);
    return fileBuffer;
}

long getLength(){
    return length;
}