#include "../global.h"
/**
 * @brief 
 * SYNTAX: PRINT Matrix
 */
bool SyntacticParsePrintMatrix()
{
    logger.log("SyntacticParsePrintMatrix");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = PRINT_MATRIX;
    parsedQuery.printRelationName = tokenizedQuery[2];
    return true;
}

bool SemanticParsePrintMatrix()
{
    logger.log("SemanticParsePrintMatrix");
    if (!matrixCatalogue.isMatrix(parsedQuery.printRelationName))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void ExecutePrintMatrix()
{
    logger.log("ExecutePrintMatrix");
    clock_t time_req = clock();
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.printRelationName);
    matrix->print();
    time_req = clock() - time_req;
    cout << "Time taken: " << (float)time_req / CLOCKS_PER_SEC << " (s)";
    return;
}
