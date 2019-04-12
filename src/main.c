#include "main.h"
#include "./parallel/encode.h"
#include "./parallel/decode.h"
#include <string.h>
#include <mpi.h>


int main(int argc, char *argv[]) {
    parseArgs(argv);

    MPI_Init(&argc, &argv);
    setupMpiVars();

    if(runType == TYPE_PARALLEL){
        if(EndsWith(inputFileName, ".drk")){
            parallelDecode(rank, numProcs, inputFileName);
        } else {
            parallelEncode(rank, numProcs, inputFileName);
        }
    }

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