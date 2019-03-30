#ifndef MAIN_HEADER
#define MAIN_HEADER

#include <stdbool.h>

    // CONST SETTINGS
    const int TYPE_SERIAL = 0;
    const int TYPE_PARALLEL = 1;

    // RUNTIME SETTINGS
    char* inputFileName;      
    int runType;

    // MPI VARIABLES
    bool master;
    int numProcs;
    int rank;


    void parseArgs(char**);
    void setupMpiVars();

#endif