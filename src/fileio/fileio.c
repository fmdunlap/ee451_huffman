#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* readIntoMemory(char* filename){
    FILE * fileToCompress;
    unsigned char *fileBuffer;

    fileToCompress = fopen(filename, "rb");

    // We can get the length of the file by jumping to the end, getting the 
    // byte offset, and then rewinding to the beginning.
    fseek(fileToCompress, 0, SEEK_END);
    length = ftell(fileToCompress);
    printf("File is of length: %li\n", length);
    rewind(fileToCompress);

    fileBuffer = (unsigned char*)malloc((length+1)*sizeof(unsigned char));
    length = fread(fileBuffer, sizeof(unsigned char), length, fileToCompress);
    printf("Read %li bytes.\n", length);
    fclose(fileToCompress);
    return fileBuffer;
}

long getLength(){
    return length;
}

char* getCompressedName(char* filename){
    char* compressedName = malloc(sizeof(char*)*sizeof(filename) + 3); // worst case we need 3 more characters for our file ending
    strcpy(compressedName, filename);
    char* dotSpot = strrchr(compressedName, '.');
    if(dotSpot == NULL){
        strcat(compressedName, ".drk");
    } else {
        *(dotSpot+1) = 'd';
        *(dotSpot+2) = 'r';
        *(dotSpot+3) = 'k';
    }
    return compressedName;
}

char* getDecompressedName(char* filename){
    char* decompressedName = malloc(sizeof(char*)*sizeof(filename) + 3); // worst case we need 3 more characters for our file ending
    strcpy(decompressedName, filename);
    char* dotSpot = strrchr(decompressedName, '.');
    if(dotSpot == NULL){
        strcat(decompressedName, ".out");
    } else {
        *(dotSpot+1) = 'o';
        *(dotSpot+2) = 'u';
        *(dotSpot+3) = 't';
    }
    return decompressedName;
}