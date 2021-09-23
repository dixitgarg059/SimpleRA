#ifndef _INCL_SEMANTICPARSER_H
#define _INCL_SEMANTICPARSER_H

#include "syntacticParser.h"

bool semanticParse();
bool semanticParseCLEAR();
bool semanticParseCROSS();
bool semanticParseDISTINCT();
bool semanticParseEXPORT();
bool semanticParseINDEX();
bool semanticParseJOIN();
bool semanticParseLIST();
bool semanticParseLOAD();
bool semanticParsePRINT();
bool semanticParsePROJECTION();
bool semanticParseRENAME();
bool semanticParseSELECTION();
bool semanticParseSORT();
bool semanticParseSOURCE();
bool SemanticParseLoadMatrix();
bool SemanticParsePrintMatrix();
bool SemanticParseExportMatrix();
bool SemanticParseTranspose();
// void hello();
#endif