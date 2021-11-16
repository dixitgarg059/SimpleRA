#ifndef _INCL_EXECUTOR_H
#define _INCL_EXECUTOR_H
#include "semanticParser.h"
void executeCommand();

void executeCLEAR();
void executeCROSS();
void executeDISTINCT();
void executeEXPORT();
void executeINDEX();
void executeJOIN();
void executeGROUPBY();
void executeLIST();
void executeLOAD();
void executePRINT();
void executePROJECTION();
void executeRENAME();
void executeSELECTION();
void executeSORT();
void executeSOURCE();
void ExecuteLoadMatrix();
void ExecutePrintMatrix();
void ExecuteExportMatrix();
void ExecuteTranspose();
bool evaluateBinOp(int value1, int value2, BinaryOperator binaryOperator);
void printRowCount(int rowCount);

#endif