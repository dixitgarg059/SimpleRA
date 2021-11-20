
#ifndef _INCL_PAGE_H
#define _INCL_PAGE_H

#include "logger.h"

/**
 * @brief The Page object is the main memory representation of a physical page
 * (equivalent to a block). The page class and the page.h header file are at the
 * bottom of the dependency tree when compiling files.
 *<p>
 * Do NOT modify the Page class. If you find that modifications
 * are necessary, you may do so by posting the change you want to make on Moodle
 * or Teams with justification and gaining approval from the TAs.
 *</p>
 */
class Page
{

public:
    // vector<int> page_data;
    string tableName;
    string pageIndex;
    int columnCount;
    int rowCount;
    vector<vector<int>> rows;
    vector<int> row;
    int cur_row;
    string pageName = "";
    Page();
    Page(const string &tableName, int pageIndex, const vector<vector<int>> &rows, int rowCount);
    Page(const string &tableName, int pageIndex);
    Page(const string &tableName, int pageIndex, const vector<int> &page_data);
    vector<int> getRow(int rowIndex);
    void writePage();
    // void WritePageForMatrix();
};

#endif