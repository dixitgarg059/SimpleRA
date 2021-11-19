#include "global.h"

bool semanticParse()
{
    logger.log("semanticParse");
    switch (parsedQuery.queryType)
    {
    case CLEAR:
        return semanticParseCLEAR();
    case CROSS:
        return semanticParseCROSS();
    case DISTINCT:
        return semanticParseDISTINCT();
    case EXPORT:
        return semanticParseEXPORT();
    case INDEX:
        return semanticParseINDEX();
    case JOIN_PART_HASH:
        return semanticParseJOIN();
    case JOIN_BLOCK_NESTED:
        return semanticParseJOIN();
    case LIST:
        return semanticParseLIST();
    case LOAD:
        return semanticParseLOAD();
    case PRINT:
        return semanticParsePRINT();
    case PROJECTION:
        return semanticParsePROJECTION();
    case RENAME:
        return semanticParseRENAME();
    case SELECTION:
        return semanticParseSELECTION();
    case SORT:
        return semanticParseSORT();
    case SOURCE:
        return semanticParseSOURCE();
    case LOAD_MATRIX:
        return SemanticParseLoadMatrix();
    case PRINT_MATRIX:
        return SemanticParsePrintMatrix();
    case EXPORT_MATRIX:
        return SemanticParseExportMatrix();
    case TRANSPOSE:
        return SemanticParseTranspose();
    case GROUP_BY:
        return semanticParseGROUPBY();
    default:
        cout << "SEMANTIC ERROR" << endl;
    }

    return false;
}
