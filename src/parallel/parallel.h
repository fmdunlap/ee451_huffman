#ifndef STANDARD_PARALLEL_HEADER
#define STANDARD_PARALLEL_HEADER

#include <mpi.h>
#include "../conf.h"

void standardParallelSubroutine(int rank, int numProcs, char* inputFileName);

#endif