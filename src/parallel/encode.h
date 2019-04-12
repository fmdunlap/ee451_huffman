#ifndef PARALLEL_ENCODE_HEADER
#define PARALLEL_ENCODE_HEADER

#include <mpi.h>
#include "../conf.h"


int serialSize;
int* serialized;

int myRank;
void parallelEncode(int rank, int numProcs, char* inputFileName);
struct MinHeapNode* buildHuffmanTreeFromOccurrences(int* occurrences);

#endif