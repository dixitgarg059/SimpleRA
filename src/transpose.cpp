#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 */

bool SyntacticParseTranspose()
{
    logger.log("SyntacticParseTranspose");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = TRANSPOSE;
    parsedQuery.exportRelationName = tokenizedQuery[1];
    return true;
}

bool SemanticParseTranspose()
{
    logger.log("SemanticParseTranspose");
    //Matrix should exist
    if (matrixCatalogue.isMatrix(parsedQuery.exportRelationName))
        return true;
    cout << "SEMANTIC ERROR: No such matrix exists" << endl;
    return false;
}

void ExecuteTranspose()
{
    logger.log("ExecuteTranspose");
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.exportRelationName);
    clock_t time_req = clock();
    matrix->transpose();
    time_req = clock() - time_req;
    cout << "Successfully transposed the matrix \n";
    cout << "Time taken: " << (float)time_req / CLOCKS_PER_SEC << " (s)";
    return;
}