#include "./parallel.h"
#include "../huffbuild/huffbuild.h"
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
            serialize analytics
        }
        broadcast serialized analytics
        deserialize analytics // TODO
        ~compress~ // TODO
        master.gather chunks // TODO
        master.writeToDisk // TODO
    */

    // Setup relevent variables.
    // Holder for number of byte occurrence in a chunk.
    long int occurrence[256];
    for(int i = 0; i < 256; i++){
        occurrence[i] = 0;
    }

    long int masterOccurrences[256];

    for(int i = 0; i < 256; i++){
        masterOccurrences[i] = 0;
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
        occurrence[(unsigned char)scatterRecv[i]]++;
    }

    //MPI_UNSIGNED_LONG worked where char and int didn't??
    MPI_Reduce(occurrence, masterOccurrences, 256, MPI_UNSIGNED_LONG, MPI_SUM, MASTER_RANK, MPI_COMM_WORLD);
    //Now that we have the occurrences, let's clean them up a bit.
    //How many of the values are non zero? These are small quick ops, so no mind that they're done serially.
    if(rank==MASTER_RANK){
        struct MinHeap* minHeap = NULL;
        struct MinHeapNode* huffmanRoot = NULL;
        int dataSize = 0;

        // Prep data to be turned into a huffman tree.
        for(int i = 0; i < 256; i++){
            if(masterOccurrences[i] != 0){
                dataSize++;
            }
        }
        int* freq = malloc(sizeof(int)*dataSize);
        char* data = malloc(sizeof(int)*dataSize);
        int j = 0;
        for(int i = 0; i < 256; i++){
            if(masterOccurrences[i] != 0){
                freq[j] = masterOccurrences[i];
                data[j] = (char)i;
                j++;
            }
        }
        
        //Build the tree and get the root.
        huffmanRoot = buildHuffmanTree(data, freq, dataSize, minHeap);

        // Get the serialized size of the tree. (Can't just use ->size b/c of
        // differing tree encodings & need to record leaves.)
        calculateSerialSize(huffmanRoot, &serialSize, 3); // 3 elements per node.

        serialized = malloc(sizeof(int)*serialSize); 
        serialize(huffmanRoot, serialized);
        // Now we have a serialized huffman tree to send out to all of the nodes! Hooray!
        // We can also use this serialization to store the tree in the file later.
    }

    MPI_Bcast(&serialSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    if(rank != MASTER_RANK){
        serialized = malloc(sizeof(int)*serialSize);
    }
    MPI_Bcast(serialized, serialSize, MPI_LONG, MASTER_RANK, MPI_COMM_WORLD);

    // Uncomment to test deserialization.
    // if(rank == 1){
    //     struct MinHeapNode* localRoot = deserialize(serialized, serialSize);
    //     int arr[MAX_TREE_HT], top = 0;
    //     printCodes(localRoot, arr, top); 
    // }

    // TODO: Encode the original file bytes
    // TODO: send blocks to master
    // TODO: merge blocks
    // TODO: Flush blocks to disk.

}
