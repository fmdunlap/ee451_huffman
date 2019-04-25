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
    MPI_Bcast(&headerDisplacement, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    MPI_Bcast(&numBytes, 1, MPI_LONG, MASTER_RANK, MPI_COMM_WORLD);

    if(myRank != MASTER_RANK){
        flushedByteLocations = malloc(sizeof(int)*origProcs);
        numFlushedBits = malloc(sizeof(int)*origProcs);
        serializedTree = malloc(sizeof(int)*serializedTreeLength);
    }

    MPI_Bcast(flushedByteLocations, origProcs, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    MPI_Bcast(numFlushedBits, origProcs, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    MPI_Bcast(serializedTree, serializedTreeLength, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    if(myRank != MASTER_RANK){
        //printHeaderDebugInfo();
    }
}

void printSendAndDisplacementDebug(){
    if(myRank == MASTER_RANK){
        dprintNoFormat("%d: ", myRank);
        dprintNoFormat("SendCounts: %d: %d", 0, sendCounts[0]);
        for(int i = 1; i < numProcs; i++){
            dprintNoFormat(", %d: %d", i, sendCounts[i]);
        }
        dprintNoFormat("\n");
        dprintNoFormat("Displacement: %d: %d", 0, displacements[0]);
        for(int i = 1; i < numProcs; i++){
            dprintNoFormat(", %d: %d", i, displacements[i]);
        }
        dprintNoFormat("\n");
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
        sendCounts[0] = numBytes - headerDisplacement;
        displacements[0] = headerDisplacement;
    }
    printSendAndDisplacementDebug();
}

void scatterFileBuffer(unsigned char* fileBuffer){
    calcSendCountsAndDisplacement();
    scatterRecv = malloc(sendCounts[myRank]*sizeof(unsigned char));
    dprint("Scattering raw file.");
    printSendAndDisplacementDebug();
    MPI_Scatterv(fileBuffer, sendCounts, displacements, MPI_UNSIGNED_CHAR, 
                scatterRecv, sendCounts[myRank], MPI_UNSIGNED_CHAR, MASTER_RANK,
                MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    dprint("Received chunk.");
}

int isFlushedByte(int relativeByteIndex){
    int absoluteByteIndex = relativeByteIndex + displacements[myRank];
    for(int i = 0; i < origProcs; i++){
        if(absoluteByteIndex == flushedByteLocations[i] - 1){
            return i;
        }
    }
    return -1;
}

// n is the number of bits to consider starting from the right.
// That is, this function sees 10011010, 8 as 1->0->0->1->1->0->1->0
// and 10100111, 3 as 1->0->1
void decodeByte(unsigned char byte, int n){
    int bitsPerByte = 8;
    for(int i = 0; i < n; i++){
        int bitToCheck = 1 << (bitsPerByte - 1 - i);
        if(byte & bitToCheck){ // if the bit at byte[bitsPerByte-1-i] == 1
            activeRoot = activeRoot->right;
        } else {
            activeRoot = activeRoot->left;
        }
        if(isLeaf(activeRoot)){
            decoded[decodedCursor] = (char) activeRoot->data;
            decodedCursor++;
            activeRoot = localRoot;
        }
    }
}

void decodeChunk(){
    //upper bounded guess, tbh. It's a project, not production. :|
    decoded = malloc(sizeof(char)*((numBytes/numProcs)*3)); // Entirely arbitrary size choice. I HIGHLY doubt that this alg will get a compression factor of anywhere near 3.
    decodedCursor = 0;
    int numBytesInBuffer = sendCounts[myRank];
    activeRoot = localRoot;

    for(int currByte = 0; currByte < numBytesInBuffer; currByte++){
        unsigned char byteVal = scatterRecv[currByte];

        int flushedIndex = isFlushedByte(currByte);
        if(flushedIndex != -1){
            dprint("I hit a flushed byte at relative %d == absolute %d. The value is: %u", currByte, currByte + displacements[myRank], scatterRecv[currByte]);
            decodeByte(byteVal, 8 - numFlushedBits[flushedIndex]);
        } else {
            decodeByte(byteVal, 8);
        }
    }

    dprint("I started/ended on absolute byte number: %d/%d", displacements[myRank], numBytesInBuffer - 1 + displacements[myRank]);
    
}

void receiveAndWriteChunks(char* inputFileName){
    long int bytes = 0;
    if(myRank != MASTER_RANK){
        MPI_Send(&decodedCursor, 1, MPI_LONG_LONG_INT, MASTER_RANK, myRank, MPI_COMM_WORLD);
        MPI_Send(&decoded, decodedCursor, MPI_UNSIGNED_CHAR, MASTER_RANK, myRank, MPI_COMM_WORLD);
    } else {
        FILE* out = fopen(getDecompressedName(inputFileName), "w+");
        for(int i = 0; i < numProcs; i++){
            unsigned char* decomp;
            long long int nBytes;
            if(i != MASTER_RANK){
                MPI_Recv(&nBytes, 1, MPI_LONG_LONG_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                decomp = malloc(sizeof(char)*nBytes);
                MPI_Recv(decomp, nBytes, MPI_UNSIGNED_CHAR, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                nBytes = decodedCursor;
                decomp = decoded;
            }
            for(int j = 0; j < nBytes; j++){
                fprintf(out, "%c", decomp[j]);
                bytes++;
            }
        }
        printf("0: Out bytes: %li", bytes);
        fclose(out);
    }
}

void parallelDecode(int rank, int nProcs, char* fName){

    /*  
        ## Decode Pseudocode ##
        MASTER:
            X Read file in
            X Compile header data
        X BCast Header Data
        X Scatter file to threads (? Maybe?)
        X Build structures/huffman tree from received header data
        X Allocate memory
        X? while(!atMyDisplacementByte){
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
    localRoot = decodeDeserialize(serializedTree, serializedTreeLength);

    decodeChunk();
    receiveAndWriteChunks(fName);

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

// The below functions are exact replicas of the ones in huffbuild.h
// I was having build issues/unexplained pointer issues and I got fed up.

// A utility function allocate a new 
// min heap node with given character 
// and frequency of the character 
struct MinHeapNode* decodeNewNode(char data, unsigned freq) 
{ 
	struct MinHeapNode* temp 
		= (struct MinHeapNode*) malloc(sizeof(struct MinHeapNode)); 

	temp->left = temp->right = NULL; 
	temp->data = data; 
	temp->freq = freq; 

	return temp; 
} 

// #PointerNightmare
void decodeDeserializeHelper(struct MinHeapNode** root, int* array, int size, int* index){
	unsigned char data = array[*index];
	unsigned freq = array[*index+1];
	int null = array[*index+2];

	*index += 3;
	if(null == 1 || *index == size - 1){
		return;
	}

	*root = decodeNewNode(data, freq);
	decodeDeserializeHelper(&((*root)->left), array, size, index);
	decodeDeserializeHelper(&((*root)->right), array, size, index);
}

// assumption of 3 ints per node.
struct MinHeapNode* decodeDeserialize(int* array, int size){
	int index = 0;
	struct MinHeapNode* root;
	decodeDeserializeHelper(&root, array, size, &index);
	return root;
}