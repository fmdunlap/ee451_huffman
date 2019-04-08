#include "./parallel.h"
#include "../huffbuild/huffbuild.h"
#include "../fileio/fileio.h"
#include <sys/types.h>
#include <sys/stat.h>

// Debug constants
#define SHOW_OCCURRENCES false
#define SHOW_CODES false
#define SHOW_DESERIALIZATION false
#define SHOW_LUT false

int bytesInBuffer = 0;
unsigned char bitBuffer = 0;
int bitsInBuffer = 0;
int flushedZeros = 0;



// Buffer one binary digit ('1' or '0')
void writeBitCharToOutput(char bitChar, unsigned char* buffer){
    if (bitsInBuffer > 7){
        buffer[bytesInBuffer] = bitBuffer;
        bitsInBuffer = 0;
        bitBuffer = 0; // just to be tidy
        bytesInBuffer++;
    }

    bitBuffer = (bitBuffer << 1) | (bitChar == '1' ? 1 : 0);
    bitsInBuffer++;
}

void writeBitStringToOutput(char* bitStr, unsigned char* buffer){
    int index = 0;
    char c = bitStr[index];
    while(c != '\0'){
        writeBitCharToOutput(c, buffer);
        index++;
        c = bitStr[index];
    }
}

// Call after last character has been encoded
// to flush out remaining bits
void flushBitBuffer(unsigned char* buffer){
    if (bitsInBuffer > 0){
        do {
            writeBitCharToOutput('0', buffer); // pad with zeroes
            flushedZeros++;
        } while (bitsInBuffer != 1);
    }
}

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
        deserialize analytics
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

    if(rank == MASTER_RANK){
        free(bigFile);
    }

    for(long int i = 0; i < sendCounts[rank]; i++){
        occurrence[(unsigned char)scatterRecv[i]]++;
    }

    //MPI_UNSIGNED_LONG worked where unsigned char and int didn't?? Weird MPI things I suppose.
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
        if(SHOW_OCCURRENCES){
            printf("------------- RANK %d OCCURRENCES BEFORE BUILDING TREE -------------\n", rank);
        }
        for(int i = 0; i < 256; i++){
            if(masterOccurrences[i] != 0){
                freq[j] = masterOccurrences[i];
                data[j] = (char)i;
                j++;
                if(SHOW_OCCURRENCES){
                    printf("%c: %ld\n",(char)i, masterOccurrences[i]);
                }
            }
        }
        
        //Build the tree and get the root.
        huffmanRoot = buildHuffmanTree(data, freq, dataSize, minHeap);
        if(SHOW_CODES){
            int arr[MAX_TREE_HT], top = 0; 
            printf("------------- RANK %d CODES BEFORE SERIALIZATION -------------\n", rank);
	        printCodes(huffmanRoot, arr, top); 
        }

        // Get the serialized size of the tree. (Can't just use ->size b/c of
        // differing tree encodings & need to record leaves.)
        calculateSerialSize(huffmanRoot, &serialSize, 3); // 3 elements per node.

        serialized = malloc(sizeof(int)*serialSize); 
        serialize(huffmanRoot, serialized);
        // Now we have a serialized huffman tree to send out to all of the nodes! Hooray!
        // We can also use this serialization to store the tree in the file later.
    }
    if(SHOW_CODES || SHOW_OCCURRENCES){
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Bcast(&serialSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    if(rank != MASTER_RANK){
        serialized = malloc(sizeof(int)*serialSize);
    }

    MPI_Bcast(serialized, serialSize, MPI_LONG, MASTER_RANK, MPI_COMM_WORLD);
    struct MinHeapNode* localRoot = deserialize(serialized, serialSize);
    if(SHOW_DESERIALIZATION){
        if(rank == 1){
            printf("------------- RANK %d DESERIALIZED HUFFMAN TREE -------------\n", rank);
            int arr[MAX_TREE_HT], top = 0;
            printCodes(localRoot, arr, top); 
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // There's definitely the argument to be made for not serializing/
    // deserialzing the tree and instead just sending each proc an LUT.
    // It's extremely valid. Going to leave serialization in for now since 
    // it doesn't take much processing. Really, though, it's just for the LUT...
    if(rank == MASTER_RANK){
        printf("LUT\n");
    }
    char** LUT = createLUT(localRoot);
    if(SHOW_LUT){
        if(rank == 1){
            printf("------------- RANK %d LUT -------------\n", rank);
            for(int i = 0; i < 256; i++){
                printf("%c: %s\n", (char) i, LUT[i]);
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    int chunkSize = 512;
    char* chunkBuff = malloc(sizeof(char)*chunkSize);
    //All but the last chunk. We can be almost certain that it's not going to divide evenly
    for(int i = 0; i < (sendCounts[rank]/chunkSize); i++){
        for(int j = 0; j < chunkSize; j++){
            chunkBuff[j] = scatterRecv[(i*chunkSize)+j];
        }
        for(int j = 0; j < chunkSize; j++){
            writeBitStringToOutput(LUT[chunkBuff[j]], scatterRecv);
        }
    }
    int startIdx = (((int)(sendCounts[rank]/chunkSize))*chunkSize);
    chunkSize = sendCounts[rank] - startIdx;
    for(int j = 0; j < chunkSize; j++){
        chunkBuff[j] = scatterRecv[startIdx + j];
    }
    for(int j = 0; j < chunkSize; j++){
        writeBitCharToOutput(chunkBuff[j], scatterRecv);
    }
    flushBitBuffer(scatterRecv);
    free(chunkBuff);

    if(rank != MASTER_RANK){
        // Send your compressed metadata and shit to the master.
        MPI_Send(&bytesInBuffer, 1, MPI_LONG_INT, MASTER_RANK, rank, MPI_COMM_WORLD);
        MPI_Send(&flushedZeros, 1, MPI_LONG_INT, MASTER_RANK, rank, MPI_COMM_WORLD);
        MPI_Send(scatterRecv, bytesInBuffer, MPI_UNSIGNED_CHAR, MASTER_RANK, rank, MPI_COMM_WORLD);
        // Free that shit stat.
        // free(scatterRecv);
    } else {
        FILE* out = fopen(getCompressedName(inputFileName), "w+");
        long int* bufferSizes = malloc(sizeof(long int)*numProcs);
        long int* numFlushedZeros = malloc(sizeof(long int)*numProcs);
        bufferSizes[0] = bytesInBuffer;
        numFlushedZeros[0] = flushedZeros;

        // Get header info from each non-master process.
        for(int i = 1; i < numProcs; i++){
            MPI_Recv(&bufferSizes[i], 1, MPI_LONG_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&numFlushedZeros[i], 1, MPI_LONG_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Master received bufferSize: %li and numFlushedZeros: %li from %d\n", bufferSizes[i], numFlushedZeros[i], i);
        }
        
        // write file header with relevant info
            // Serialized huffman
            // Chunk locations & num zeros.
            // ???
        fprintf(out, "-\n");
        for(int i = 0; i < numProcs; i++){ 
            fprintf(out, "%d\n", bufferSizes[i]);
        }
        fprintf(out, "--\n");
        for(int i = 0; i < numProcs; i++){
            fprintf(out, "%d\n", numFlushedZeros[i]);
        }
        fprintf(out, "---\n");
        for(int i = 0; i < numProcs; i++){
            unsigned char* comp;
            if(i != MASTER_RANK){
                comp = (unsigned char*)malloc(sizeof(unsigned char)*bufferSizes[i]);
                printf("Attempting to allocate %li bytes of mem.\n", bufferSizes[i]);

                printf("Receiving compressed from proc %d.\n", i);
                MPI_Recv(comp, bufferSizes[i], MPI_UNSIGNED_CHAR, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Received compressed from proc %d.\n", i);
            } else {
                printf("Exporting master contents.\n");                
                comp = scatterRecv;
            }
            for(int j = 0; j < bufferSizes[i]; j++){
                fprintf(out, "%c", comp[j]);
            }
            printf("Exported compressed contents of proc %d.\n", i);    
            free(comp);
        }
            // if !master_rank
                // recv compressed info
            // else 
                // stage master compressed info
            // write rank's compressed shit to file.
        //close file
        fclose(out);
    }
    // merge blocks -- this is going to be tricky.
    // Current thought is to write the info to the file header, where we store the 
    // serialized tree/LUT. Logic as follows
        // Proc gets to end of encoding sequence and has x bits left in a byte
        
        // Flushes n zeros at the end of encodedByte e so that we have a full 8
        // bit byte.

        // First tells master "There are n zeros appended at the end 
        // of byte e in my encoded chunk."

        // Master collects these messages and writes these into the file header
        // along with the LUT/serialized Huffman Tree

        // Master writes blocks in order to disk as it received them.

        // On decompress, it removes those 'n' bits from the byte while
        // doing the huffman decode.
}
