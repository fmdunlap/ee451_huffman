#include "./parallel.h"
#include "../huffbuild/huffbuild.h"
#include "../fileio/fileio.h"
#include <sys/types.h>
#include <sys/stat.h>

// Debug constants
#define SHOW_VERBOSE_MESSAGES true
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

void dprint(char* msg){
    if(SHOW_VERBOSE_MESSAGES)
        printf("%d: %s\n", myRank, msg);
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
        ~compress~
        master.gather chunks
        master.writeToDisk
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
    myRank = rank;

    // "You may sit on this council, but you are not granted the rank of master."
    if(myRank == MASTER_RANK){
        dprint("Master reading file into memory.");
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

    dprint("Calculating send counts and displacement.");
    if(numProcs > 1){
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
    } else {
        sendCounts[0] = numBytes;
        displacement[0] = 0;
    }

    unsigned char* scatterRecv = malloc(sendCounts[myRank]*sizeof(unsigned char));

    dprint("Scattering raw file.");
    MPI_Scatterv(bigFile, sendCounts, displacement, MPI_UNSIGNED_CHAR, 
                scatterRecv, sendCounts[myRank], MPI_UNSIGNED_CHAR, MASTER_RANK,
                MPI_COMM_WORLD);

    if(myRank == MASTER_RANK){
        free(bigFile);
        dprint("File memory freed.");
    }

    dprint("Counting character occurrences.");
    for(long int i = 0; i < sendCounts[myRank]; i++){
        occurrence[(unsigned int)scatterRecv[i]]++;
    }

    
    dprint("Reducing character occurrences to master.");
    //MPI_UNSIGNED_LONG worked where unsigned char and int didn't?? Weird MPI things I suppose.
    MPI_Reduce(occurrence, masterOccurrences, 256, MPI_UNSIGNED_LONG, MPI_SUM, MASTER_RANK, MPI_COMM_WORLD);

    //Now that we have the occurrences, let's clean them up a bit.
    //How many of the values are non zero? These are small quick ops, so no mind that they're done serially.
    if(myRank==MASTER_RANK){
        dprint("Master building huffman tree.");
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
            dprint("------------- OCCURRENCES BEFORE BUILDING TREE -------------\n");
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
            dprint("------------- CODES BEFORE SERIALIZATION -------------\n");
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

    dprint("Broadcasting serialized huffman tree size.");
    MPI_Bcast(&serialSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    if(myRank != MASTER_RANK){
        serialized = malloc(sizeof(int)*serialSize);
    }

    dprint("Broadcasting serialized huffman tree.");
    MPI_Bcast(serialized, serialSize, MPI_LONG, MASTER_RANK, MPI_COMM_WORLD);
    struct MinHeapNode* localRoot = deserialize(serialized, serialSize);
    if(SHOW_DESERIALIZATION){
        if(myRank == 1){
            dprint("------------- DESERIALIZED HUFFMAN TREE -------------\n");
            int arr[MAX_TREE_HT], top = 0;
            printCodes(localRoot, arr, top); 
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // There's definitely the argument to be made for not serializing/
    // deserialzing the tree and instead just sending each proc an LUT.
    // It's extremely valid. Going to leave serialization in for now since 
    // it doesn't take much processing. Really, though, it's just for the LUT...
    char** LUT = createLUT(localRoot);
    if(SHOW_LUT){
        if(myRank == 1){
            dprint("------------- LUT -------------\n");
            for(int i = 0; i < 256; i++){
                printf("%c: %s\n", (char) i, LUT[i]);
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    int chunkSize = 4096;
    unsigned char* compressed = malloc(sizeof(unsigned char)*numBytes);
    dprint("Allocated compressed file.");
    unsigned char* chunkBuff = malloc(sizeof(unsigned char)*chunkSize);
    dprint("Allocated chunk buff.");
    MPI_Barrier(MPI_COMM_WORLD);
    //All but the last chunk. We can be almost certain that it's not going to divide evenly
    for(int i = 0; i < (sendCounts[myRank]/chunkSize); i++){
        for(int j = 0; j < chunkSize; j++){
            chunkBuff[j] = scatterRecv[(i*chunkSize)+j];
        }
        for(int j = 0; j < chunkSize; j++){
            writeBitStringToOutput(LUT[(unsigned int)chunkBuff[j]], compressed);
        }
    }
    dprint("Calculating start index.");
    int startIdx = (((int)(sendCounts[myRank]/chunkSize))*chunkSize);
    chunkSize = sendCounts[myRank] - startIdx;
    for(int j = 0; j < chunkSize; j++){
        chunkBuff[j] = scatterRecv[startIdx + j];
    }
    for(int j = 0; j < chunkSize; j++){
        writeBitCharToOutput(chunkBuff[j], compressed);
    }
    flushBitBuffer(compressed);
    free(scatterRecv);
    free(chunkBuff);

    if(myRank != MASTER_RANK){
        // Send your compressed metadata and shit to the master.
        MPI_Send(&bytesInBuffer, 1, MPI_LONG_INT, MASTER_RANK, myRank, MPI_COMM_WORLD);
        MPI_Send(&flushedZeros, 1, MPI_LONG_INT, MASTER_RANK, myRank, MPI_COMM_WORLD);
        MPI_Send(compressed, bytesInBuffer, MPI_UNSIGNED_CHAR, MASTER_RANK, myRank, MPI_COMM_WORLD);
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
                comp = compressed;
            }
            for(int j = 0; j < bufferSizes[i]; j++){
                fprintf(out, "%c", comp[j]);
            }
            dprint("Exported compressed contents of proc.");
            dprint("Freeing comp.");
            free(comp);
        }
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
