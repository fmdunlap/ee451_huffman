#ifndef STANDARD_PARALLEL_HEADER
#define STANDARD_PARALLEL_HEADER

#include <mpi.h>
#include "../conf.h"


int serialSize;
int* serialized;

int myRank;

void dprint(char* msg);
void standardParallelSubroutine(int rank, int numProcs, char* inputFileName);
struct MinHeapNode* buildHuffmanTreeFromOccurrences(int* occurrences);

#endif