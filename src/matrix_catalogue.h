
#ifndef _INCL_MATRIX_CATALOGUE_H
#define _INCL_MATRIX_CATALOGUE_H
#include "matrix.h"

// using namespace std;
/**
 * @brief The MatrixCatalogue acts like an index of matrices existing in the
 * system. Everytime a matrix is added(removed) to(from) the system, it needs to
 * be added(removed) to(from) the MatrixCatalogue. 
 *
 */
class MatrixCatalogue
{

    unordered_map<string, Matrix *> matrices;

public:
    MatrixCatalogue() {}
    void insertMatrix(Matrix *matrix);
    void deleteMatrix(const string &matrix_name);
    Matrix *getMatrix(const string &matrix_name);
    bool isMatrix(const string &matrix_name);
    void print();
    ~MatrixCatalogue();
};
#endif