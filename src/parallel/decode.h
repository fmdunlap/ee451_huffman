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

// Communicated data
long numBytes;
int* flushedByteLocations;
int* numFlushedBits;
int* serializedTree;
int serializedTreeLength;

//Master Only Data
int origProcs;
int headerDisplacement;


void parallelDecode(int rank, int numProcs, char* inputFileName);
void readBuffLine(unsigned char* readFrom, char* buff, long int* bufferLocation);
int readHeader(unsigned char* fileBuffer);
void printHeaderDebugInfo();
void bcastHeader();
void calcSendCountsAndDisplacement();
void scatterFileBuffer();


#endif