#include "./parallel.h"
#include "../fileio/fileio.h"
#include <sys/types.h>
#include <sys/stat.h>

void standardParallelSubroutine(int rank, int numProcs, char* inputFileName){
    /* psuedocode:
        if(master){
            readInFile()
        }
        scatter relevant chunks
        do statistical analysis on recieved chunk
        master.gather statistical data
        if(master){
            compute analytics
        }
        broadcast analytics
        ~compress~
        master.gather chunks
        master.writeToDisk
    */

    // Setup relevent variables.
        // Holder for number of byte occurences in a chunk.
    long int occurences[256];
    for(int i = 0; i < 256; i++){
        occurences[i] = 0;
    }
    long int masterOccurences[256];
    for(int i = 0; i < 256; i++){
        masterOccurences[i] = 0;
    }
    unsigned char* bigFile = NULL;
    long numBytes;

    // "You may sit on this council, but you are not granted the rank of master."
    if(rank == MASTER_RANK){
        bigFile = readIntoMemory(inputFileName);
        numBytes = getLength();
    }

    // First, tell everyone how many ints there will be...
    MPI_Bcast(&numBytes, 1, MPI_LONG, MASTER_RANK, MPI_COMM_WORLD);

    //Some helpful bits of derived data
    // C truncates doubles when casting to int which is effectively a floor.
    long int bytesPerProc = numBytes/numProcs; 

    //Build send counts array (naive for now).
    int sendCounts[numProcs];
    int displacement[numProcs];

    for(int i = 0; i < numProcs-1; i++){
        sendCounts[i] = bytesPerProc;
        if(i != 0){
            displacement[i] = displacement[i-1] + bytesPerProc;
        } else {
            displacement[i] = 0;
        }
    }
    sendCounts[numProcs-1] = numBytes - bytesPerProc*(numProcs-1);
    displacement[numProcs-1] = displacement[numProcs-2] + bytesPerProc;

    unsigned char* scatterRecv = malloc(sendCounts[rank]*sizeof(unsigned char));

    MPI_Scatterv(bigFile, sendCounts, displacement, MPI_UNSIGNED_CHAR, 
                scatterRecv, sendCounts[rank], MPI_UNSIGNED_CHAR, MASTER_RANK,
                MPI_COMM_WORLD);

    for(long int i = 0; i < sendCounts[rank]; i++){
        occurences[(unsigned char)scatterRecv[i]]++;
    }

    MPI_Reduce(occurences, masterOccurences, 256, MPI_UNSIGNED_LONG, MPI_SUM, MASTER_RANK, MPI_COMM_WORLD);

    if(rank == MASTER_RANK){
        for(int i = 0; i < 256; i++){
            printf("%li\n", masterOccurences[i]);
        }
    }

}
