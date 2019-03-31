#ifndef MAIN_HEADER
#define MAIN_HEADER

#include "./conf.h"

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