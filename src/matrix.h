

#ifndef _INCL_MATRIX_H
#define _INCL_MATRIX_H

#include "bufferManager.h"

//TODO: change it
/**
 * @brief The Matrix class holds all information related to a loaded Matrix. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a Matrix object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (SELECT, PROJECT,
 * JOIN, SORT, CROSS and DISTINCT). 

 */
class Matrix
{
    // vector<unordered_set<int>> distinctValuesInColumns;

public:
    bool is_sparse = false;
    string sourceFileName = "";
    string MatrixName = "";
    vector<string> columns;
    // vector<uint> distinctValuesPerColumnCount;
    uint columnCount = 0;
    uint rowCount = 0;
    uint blockCount = 0;
    uint sub_matrix_side = 0;
    uint maxRowsPerBlock = 0;
    // vector<uint> rowsPerBlockCount;
    // bool indexed = false;
    // string indexedColumn = "";
    // IndexingStrategy indexingStrategy = NOTHING;

    // bool extractColumnNames(string firstLine);
    bool blockify();
    // void updateStatistics(vector<int> row);
    Matrix();
    Matrix(string MatrixName);
    // Matrix(string MatrixName, vector<string> columns);
    bool load();
    // bool isColumn(string columnName);
    void renameColumn(string fromColumnName, string toColumnName);
    void print();
    void makePermanent();
    bool CheckIfSparse();
    bool isPermanent();
    bool blockify_for_sparse();
    int GetPageNumberFromCoordinates(int row, int col);
    // void getNextPage(Cursor *cursor);
    Cursor getCursor();
    int getColumnIndex(string columnName);
    void unload();
    void transpose();
    int FindInCompressedTable(int row, int col);
    /**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
    template <typename T>
    void writeRow(const vector<T> &row, ostream &fout)
    {
        logger.log("Matrix::printRow");
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
        }
        fout << endl;
    }

    /**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
    template <typename T>
    void writeRow(const vector<T> &row)
    {
        logger.log("Matrix::printRow");
        ofstream fout(this->sourceFileName, ios::app);
        this->writeRow(row, fout);
        fout.close();
    }
};
#endif