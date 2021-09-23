#include "global.h"
/**
 * @brief 
 * SYNTAX: LOAD relation_name
 */
bool SyntacticParseLoadMatrix()
{
    logger.log("SyntacticParseLoadMatrix");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LOAD_MATRIX;
    parsedQuery.loadRelationName = tokenizedQuery[2];
    return true;
}

bool SemanticParseLoadMatrix()
{
    logger.log("SemanticParseLoadMatrix");
    if (tableCatalogue.isTable(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Matrix already exists" << endl;
        return false;
    }

    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }
    return true;
}

void ExecuteLoadMatrix()
{
    logger.log("executeLOADMatrix");

    Matrix *matrix = new Matrix(parsedQuery.loadRelationName);
    clock_t time_req = clock();
    if (matrix->load())
    {
        time_req = clock() - time_req;
        matrixCatalogue.insertMatrix(matrix);
        cout << "Loaded Matrix Column Count: " << matrix->columnCount << " Row Count: " << matrix->rowCount << endl;
        cout << "Time taken: " << (float)time_req / CLOCKS_PER_SEC << " (s)";
    }
    return;
}
