#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 */

bool SyntacticParseExportMatrix()
{
    logger.log("SyntacticParseExportMatrix");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = EXPORT_MATRIX;
    parsedQuery.exportRelationName = tokenizedQuery[2];
    return true;
}

bool SemanticParseExportMatrix()
{
    logger.log("SemanticParseExportMatrix");
    //Matrix should exist
    if (matrixCatalogue.isMatrix(parsedQuery.exportRelationName))
        return true;
    cout << "SEMANTIC ERROR: No such matrix exists" << endl;
    return false;
}

void ExecuteExportMatrix()
{
    logger.log("ExecuteExportMatrix");
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.exportRelationName);
    clock_t time_req = clock();
    matrix->makePermanent();
    time_req = clock() - time_req;
    cout << "Successfully exported the matrix \n";
    cout << "Time taken: " << (float)(time_req) / CLOCKS_PER_SEC << " (s)";
    return;
}