#ifndef PARALLEL_DECODE_HEADER
#define PARALLEL_DECODE_HEADER

#include "../conf.h"
#include <mpi.h>

// Passed in data
int myRank;
int numProcs;
char* inputFileName;

// Local data
int* sendCounts;
int* displacements;
unsigned char* scatterRecv;
unsigned char* decoded; // Where we hold onto our decoded text. Global because I don't feel like dealing with pointers.
long long int decodedCursor;

// Communicated data
long numBytes;
int* flushedByteLocations;
int* numFlushedBits;
int* serializedTree;
int serializedTreeLength;
int origProcs;
int headerDisplacement;

// Huffman Tree data (perThread)
struct MinHeapNode { 
	// One of the input characters 
	unsigned char data; 

	// Frequency of the character 
	unsigned freq; 

	// Left and right child of this node 
	struct MinHeapNode *left, *right; 
};
struct MinHeapNode* localRoot;
struct MinHeapNode* activeRoot;
struct MinHeapNode* decodeNewNode(char data, unsigned freq);
void decodeDeserializeHelper(struct MinHeapNode** root, int* array, int size, int* index);
struct MinHeapNode* decodeDeserialize(int* array, int size);

void parallelDecode(int rank, int numProcs, char* inputFileName);
void readBuffLine(unsigned char* readFrom, char* buff, long int* bufferLocation);
int readHeader(unsigned char* fileBuffer);
void printHeaderDebugInfo();
void bcastHeader();
void calcSendCountsAndDisplacement();
void scatterFileBuffer();


#endif