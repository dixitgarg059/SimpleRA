#include "global.h"

namespace
{

    void WritePageHelper(const Page *p, ostream &fout)
    {
        for (int rowCounter = 0; rowCounter < p->rowCount; rowCounter++)
        {
            for (int columnCounter = 0; columnCounter < p->columnCount; columnCounter++)
            {
                if (columnCounter != 0)
                {
                    cout << " ";
                    fout << " ";
                }
                fout << p->rows[rowCounter][columnCounter];
                // cout << p->rows[rowCounter][columnCounter];
            }
            if (rowCounter != p->rowCount)
            {
                fout << "\n";
                // cout << endl;
            }
        }
    }

}

/**
 * @brief Construct a new Page object. Never used as part of the code
 *
 *
 *
 */
Page::Page()
{
    this->pageName = "";
    this->tableName = "";
    this->pageIndex = -1;
    this->rowCount = 0;
    this->columnCount = 0;
    this->rows.clear();
    this->row.clear();
}
/**
 * @brief Construct a new Page:: Page object given the table name and page
 * index. When tables are loaded they are broken up into blocks of BLOCK_SIZE
 * and each block is stored in a different file named
 * "<tablename>_Page<pageindex>". For example, If the Page being loaded is of
 * table "R" and the pageIndex is 2 then the file name is "R_Page2". The page
 * loads the rows (or tuples) into a vector of rows (where each row is a vector
 * of integers).
 *
 * @param tableName
 * @param pageIndex
 */
Page::Page(const string &tableName, int pageIndex)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
    Table *table = tableCatalogue.getTable(tableName);
    this->columnCount = table->columnCount;
    uint maxRowCount = table->maxRowsPerBlock;
    vector<int> row(columnCount, 0);
    this->rows.assign(maxRowCount, row);

    ifstream fin(pageName, ios::in);
    this->rowCount = table->rowsPerBlockCount[pageIndex];
    int number;
    for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
        {
            fin >> number;
            this->rows[rowCounter][columnCounter] = number;
        }
    }
    fin.close();
}

/**
 * @brief Get row from page indexed by rowIndex
 *
 * @param rowIndex
 * @return vector<int>
 */
vector<int> Page::getRow(int rowIndex)
{
    logger.log("Page::getRow");
    vector<int> result;
    result.clear();
    if (rowIndex >= this->rowCount)
        return result;
    return this->rows[rowIndex];
}

Page::Page(const string &tableName, int pageIndex, const vector<vector<int>> &rows, int rowCount)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->rows = rows;
    this->rowCount = rowCount;
    if (rows.size() > 0)
        this->columnCount = rows[0].size();
    else
        this->columnCount = 0;
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
}

/**
 * @brief writes current page contents to file.
 *
 */

// void Page::WritePageForMatrix()
// {

//     logger.log("Page::WritePageForMatrix");
//     ofstream fout(this->pageName, ios::trunc);
//     for (auto data : this->page_data)
//     {
//         fout << data << " ";
//     }
//     fout.close();
// }

void Page::writePage()
{
    logger.log("Page::writePage");
    ofstream fout(this->pageName, ios::trunc);
    WritePageHelper(this, fout);
    fout.close();
    block_accesses_write++;
    // cout << "writing page " << this->pageName << endl;
}

void Page::writePage(int ch)
{
    logger.log("Page::writePage");
    ofstream fout(this->pageName, (ch ? ios::app : ios::trunc));
    WritePageHelper(this, fout);
    fout.close();
    block_accesses_write++;
}

// Page::Page(string tableName, int pageIndex, vector<int> page_data)
// {

//     logger.log("Page::Page for matrix");
//     this->tableName = tableName;
//     this->pageIndex = pageIndex;
//     // this->rows = ;
//     // this->rowCount = 1;
//     // this->columnCount = page_data.size();
//     this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
//     this->page_data = page_data;
// }
