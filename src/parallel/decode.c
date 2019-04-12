#include "./decode.h"
#include "../fileio/fileio.h"
#include <string.h>
#include <stdarg.h>

// Debug constants
#define DECODE_SHOW_VERBOSE_MESSAGES true

static void dprint(const char* fmt, ...){
    if(DECODE_SHOW_VERBOSE_MESSAGES){
        va_list args;
        printf("%d: ", myRank);
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
    }
}

static void dprintNoFormat(const char* fmt, ...){
    if(DECODE_SHOW_VERBOSE_MESSAGES){
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}

int readHeader(unsigned char* fileBuffer){
    long int buffLoc = 0;
    
    // All of this ugliness is just reading in the headers so that we can use them.
    //arbitrarily large because idgaf
    char buff[10000];

    readBuffLine(fileBuffer, buff, &buffLoc);
    origProcs = atoi(buff);

    //allocate enough memory for the number of prcs.
    flushedByteLocations = malloc(sizeof(long int)*origProcs);
    numFlushedBits = malloc(sizeof(long int)*origProcs);
    int originalByteLocations[origProcs];

    readBuffLine(fileBuffer, buff, &buffLoc); // move past the '-' marker.

    // Read in where each processor had its flushed byte, store since we want
    // to calc displacement after we know length of header.
    for(int i = 0; i < origProcs; i++){
        readBuffLine(fileBuffer, buff, &buffLoc);
        originalByteLocations[i] = atoi(buff);
    }

    readBuffLine(fileBuffer, buff, &buffLoc); // move past the '-' marker.
    
    // Read in array of length of flushed byte.
    for(int i = 0; i < origProcs; i++){
        readBuffLine(fileBuffer, buff, &buffLoc);
        int numFlushed = atoi(buff);
        numFlushedBits[i] = numFlushed;
    }

    readBuffLine(fileBuffer, buff, &buffLoc); // move past the '-' marker.
    readBuffLine(fileBuffer, buff, &buffLoc); // read in length of serialized array so we can allocate necessary memory.
    
    serializedTreeLength = atoi(buff); // Will need this value later when we're communicating this array.
    serializedTree = malloc(sizeof(int)*serializedTreeLength);

    readBuffLine(fileBuffer, buff, &buffLoc); // read in the serialized tree.

    // Detokenize the tree using commas and read it into serializedTree
    char* t = strtok(buff, ",");
    int treeIndex = 0;
    while (t != NULL) {
        int serialEntry = atoi(t);
        serializedTree[treeIndex] = serialEntry;
        treeIndex++;
        t = strtok(NULL, ","); 
    }

    readBuffLine(fileBuffer, buff, &buffLoc); // Move the buffLoc past the last '-'

    // Set displacement to buffer location since that's the beginning of the actual file.
    // This way all byte locations are absolute.
    int flushedByteDisplacement = buffLoc;
    dprint("Displacement: %d", flushedByteDisplacement);
    for(int i = 0; i < origProcs; i++){
        int byteLoc = originalByteLocations[i];
        flushedByteDisplacement += byteLoc;
        flushedByteLocations[i] = flushedByteDisplacement;
        dprint("Displacement: %d", flushedByteDisplacement);
    }

    return buffLoc;
}

void printHeaderDebugInfo(){
    dprint("Orig procs: %d", origProcs);
    
    for(int i = 0; i < origProcs; i++){
        dprint("%d's flushed byte location: %lli", i, flushedByteLocations[i]);
        dprint("%d's number of flushed bits: %d", i, numFlushedBits[i]);
    }

    dprint("Serialized Tree: ");
    for(int i = 0; i < serializedTreeLength - 1; i++){
        dprintNoFormat("%d,", serializedTree[i]);
    }
    dprintNoFormat("%d\n", serializedTree[serializedTreeLength - 1]);
}

void bcastHeader(){
    MPI_Bcast(&origProcs, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    MPI_Bcast(&serializedTreeLength, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    if(myRank != MASTER_RANK){
        flushedByteLocations = malloc(sizeof(int)*origProcs);
        numFlushedBits = malloc(sizeof(int)*origProcs);
        serializedTree = malloc(sizeof(int)*serializedTreeLength);
    }

    MPI_Bcast(flushedByteLocations, origProcs, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    MPI_Bcast(numFlushedBits, origProcs, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    MPI_Bcast(serializedTree, serializedTreeLength, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    if(myRank != MASTER_RANK){
        printHeaderDebugInfo();
    }
}

void printSendAndDisplacementDebug(){
    if(myRank == MASTER_RANK){
        dprint("Send counts");
        for(int i = 0; i < numProcs; i++){
            dprint("%d: %d", i, sendCounts[i]);
        }
        dprint("Displacements");
        for(int i = 0; i < numProcs; i++){
            dprint("%d: %d", i, displacements[i]);
        }
    }
}

void calcSendCountsAndDisplacement(){
    long int bytesPerProc = (numBytes-headerDisplacement)/numProcs;
    sendCounts = malloc(sizeof(int)*numProcs);
    displacements = malloc(sizeof(int)*numProcs);


    if(numProcs > 1){
        for(int i = 0; i < numProcs-1; i++){
            sendCounts[i] = bytesPerProc;
            if(i != 0){
                displacements[i] = displacements[i-1] + bytesPerProc;
            } else {
                displacements[i] = headerDisplacement;
            }
        }
        sendCounts[numProcs-1] = numBytes - bytesPerProc*(numProcs-1) - headerDisplacement;
        displacements[numProcs-1] = displacements[numProcs-2] + bytesPerProc;
    } else {
        sendCounts[0] = numBytes;
        displacements[0] = headerDisplacement;
    }
    printSendAndDisplacementDebug();
}

void scatterFileBuffer(unsigned char* fileBuffer){
    calcSendCountsAndDisplacement();
    scatterRecv = malloc(sendCounts[myRank]*sizeof(unsigned char));
    MPI_Barrier(MPI_COMM_WORLD);
    // dprint("Scattering raw file.");
    // MPI_Scatterv(fileBuffer, sendCounts, displacements, MPI_UNSIGNED_CHAR, 
    //             scatterRecv, sendCounts[myRank], MPI_UNSIGNED_CHAR, MASTER_RANK,
    //             MPI_COMM_WORLD);
    // dprint("Received chunk.");
}

void parallelDecode(int rank, int nProcs, char* fName){

    /*  
        ## Decode Pseudocode ##
        MASTER:
            X Read file in
            X Compile header data & broadcast
        X BCast Header Data
        TODO Scatter file to threads
        TODO Build structures/huffman tree from received header data
        TODO Receive scattered files.
        TODO Allocate memory
        TODO while(!atMyDisplacementByte){
            decodeWithHuffmanTree
        }
        TODO Send Decoded chunks to master
        MASTER:
            TODO write file to disk using received chunks.
    */
    
    myRank = rank;
    numProcs = nProcs;
    char* inputFileName = fName;
    unsigned char* bigFile = NULL; //https://i.ytimg.com/vi/YFXlzKDjL8E/maxresdefault.jpg

    if(myRank == MASTER_RANK){
        bigFile = readIntoMemory(inputFileName);
        numBytes = getLength();

        headerDisplacement = readHeader(bigFile);
        printHeaderDebugInfo();
    }
    
    bcastHeader();
    scatterFileBuffer(bigFile);

    return;
}

void readBuffLine(unsigned char* readFrom, char* buff, long int* bufferLocation){
    int i = 0;
    while(readFrom[*bufferLocation] != '\n'){
        buff[i] = readFrom[*bufferLocation];
        i++;
        (*bufferLocation)++;
    }
    buff[i] = '\0';
    (*bufferLocation)++;
}