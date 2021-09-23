#ifndef _INCL_CURSOR_H
#define _INCL_CURSOR_H

#include "bufferManager.h"
/**
 * @brief The cursor is an important component of the system. To read from a
 * table, you need to initialize a cursor. The cursor reads rows from a page one
 * at a time.
 *
 */
class Cursor
{
public:
    Page page;
    int pageIndex;
    string tableName;
    // pagePointer -> which row in a particular page
    int pagePointer;

public:
    Cursor(string tableName, int pageIndex);
    vector<int> getNext();
    // vector<int> GetNextForMatrix();
    void nextPage(int pageIndex);
};

#endif