#ifndef MAIN_HEADER
#define MAIN_HEADER

#include "./conf.h"
#include <string.h>

// RUNTIME SETTINGS
char* inputFileName;      
int runType;

// MPI VARIABLES
bool master;
int numProcs;
int rank;


void parseArgs(char**);
void setupMpiVars();

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

#endif