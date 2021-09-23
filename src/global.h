#ifndef _INCL_GLOBAL_H
#define _INCL_GLOBAL_H
#include "executor.h"

#define uint unsigned int
extern float BLOCK_SIZE;
extern uint BLOCK_COUNT;
extern uint PRINT_COUNT;
extern std::vector<std::string> tokenizedQuery;
extern ParsedQuery parsedQuery;
extern TableCatalogue tableCatalogue;
extern BufferManager bufferManager;
extern MatrixCatalogue matrixCatalogue;

#endif