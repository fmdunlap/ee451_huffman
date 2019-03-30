#include "main.h"
#include "./fileio/fileio.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
    printf("Working like magic. \n");
    parseArgs(argv);

    MPI_Init(&argc, &argv);
    setupMpiVars();

    

    MPI_Finalize();
    return 0;
}

void setupMpiVars(){
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(rank == 0){
        master = true;
    }
}

// Arg parsing is simplified since we're handling
// the execution flow from the run script.
// IE: [1] => runtype (s/p), [2] => input file location
void parseArgs(char* argvals[]){
    if(strcmp(argvals[1], "s") == 0){
        runType = TYPE_SERIAL;
    } else {
        runType = TYPE_PARALLEL;
    }

    inputFileName = argvals[2];
}