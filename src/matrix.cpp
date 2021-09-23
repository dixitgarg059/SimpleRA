#include "global.h"
#include "matrix.h"

namespace
{
    int cel(int x1, int y1)
    {
        if ((x1 % y1) == 0)
            return x1 / y1;
        else
            return x1 / y1 + 1;
    }
    void SwapPages(Page *page_1, Page *page_2)
    {
        if (page_1 == page_2)
        {
            for (int i = 0; i < page_1->rowCount; ++i)
            {
                for (int j = i + 1; j < page_1->columnCount; ++j)
                {
                    swap(page_1->rows[i][j], page_2->rows[j][i]);
                }
            }
        }
        else
        {
            for (int i = 0; i < page_1->rowCount; ++i)
            {
                for (int j = 0; j < page_1->columnCount; ++j)
                {

                    swap(page_1->rows[i][j], page_2->rows[j][i]);
                }
            }
        }
    }
    int FindInPage(Page *page, int row, int col)
    {

        int l = 0, f = page->rows.size() - 1;
        int lower = -1, upper = -1;
        while (l <= f)
        {
            int mid = l + (f - l) / 2;
            if (page->rows[mid][0] > col)
            {
                f = mid - 1;
            }
            else if (page->rows[mid][0] < col)
            {
                l = mid + 1;
            }
            else
            {
                lower = mid;
                f = mid - 1;
            }
        }
        l = 0, f = page->rows.size() - 1;
        while (l <= f)
        {
            int mid = l + (f - l) / 2;
            if (page->rows[mid][0] > col)
            {
                f = mid - 1;
            }
            else if (page->rows[mid][0] < col)
            {
                l = mid + 1;
            }
            else
            {
                upper = mid;
                l = mid + 1;
            }
        }
        if (lower == -1)
        {
            // not found
            return 0;
        }

        // rows will be in sorted order in the range [lower,upper] => binary search

        l = lower, f = upper;
        while (l <= f)
        {
            int mid = l + (f - l) / 2;

            if (page->rows[mid][1] < row)
            {
                l = mid + 1;
            }
            else if (page->rows[mid][1] > row)
            {
                f = mid - 1;
            }
            else
            {
                // found
                return page->rows[mid][2];
            }
        }
        // not found
        return 0;
    }
}
/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param MatrixName 
 */
Matrix::Matrix(string MatrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + MatrixName + ".csv";
    this->MatrixName = MatrixName;
}

// /**
//  * @brief Construct a new Matrix:: Matrix object used when an assignment command
//  * is encountered. To create the Matrix object both the Matrix name and the
//  * columns the Matrix holds should be specified.
//  *
//  * @param MatrixName
//  * @param columns
//  */
// // Matrix::Matrix(string MatrixName, vector<string> columns)
// // {
// //     logger.log("Matrix::Matrix");
// //     this->sourceFileName = "../data/temp/" + MatrixName + ".csv";
// //     this->MatrixName = MatrixName;
// //     this->columns = columns;
// //     this->columnCount = columns.size();
// //     this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
// //     this->writeRow<string>(columns);
// // }

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates Matrix
 * statistics.
 *
 * @return true if the Matrix has been successfully loaded 
 * @return false if an error occurred 
 */

bool Matrix::CheckIfSparse()
{

    // return true;
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    int count_zero = 0, count_total = 0;
    while (getline(fin, word, ','))
    {
        if (find(word.begin(), word.end(), '\n') != word.end())
        {
            stringstream s(word);
            string word1;

            while (getline(s, word1))
            {
                int num = stoi(word1);
                count_total++;
                if (num == 0)
                    count_zero++;
            }
        }
        else
        {
            int num = stoi(word);
            count_total++;
            if (num == 0)
                count_zero++;
        }
    }
    fin.close();
    double percentage = static_cast<double>(count_zero) / static_cast<double>(count_total);
    this->rowCount = sqrt(count_total);
    this->columnCount = this->rowCount;
    return percentage >= 0.6;
}
bool Matrix::load()
{
    logger.log("Matrix::load");
    if (CheckIfSparse())
    {
        this->is_sparse = true;
        return this->blockify_for_sparse();
    }
    else
    {
        this->is_sparse = false;
        return this->blockify();
    }
}

bool Matrix::blockify_for_sparse()
{

    this->columns.push_back("rows");
    this->columns.push_back("columns");
    this->columns.push_back("values");
    this->blockCount = 0;
    this->maxRowsPerBlock = (1000 * BLOCK_SIZE) / (3 * sizeof(int));
    ifstream fin(this->sourceFileName, ios::in);

    string line;
    // vector<int> row(this->columnCount, 0);
    // a page/blockt
    vector<vector<int>> rowsInPage;
    rowsInPage.reserve(maxRowsPerBlock);
    // int rowCounter = 0;
    // unordered_set<int1> dummy;;
    // dummy.clear();
    // this->distinctValuesInColumns.assign(this->columnCount, dummy);
    // this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    // getline(fin, line);
    int row = 0;
    string word;
    for (int row = 0; row < this->rowCount; ++row)
    {

        for (int col = 0; col < this->columnCount; ++col)
        {
            if (col == this->columnCount - 1)
                getline(fin, word);
            else
                getline(fin, word, ',');
            int value = stoi(word);
            if (value != 0)
            {
                vector<int> row_data = {row, col, value};
                rowsInPage.push_back(row_data);
                if (rowsInPage.size() == this->maxRowsPerBlock)
                {
                    bufferManager.writePage(this->MatrixName, /*page number = */ this->blockCount, /*data = */ rowsInPage, rowsInPage.size());
                    this->blockCount++;
                    // this->rowsPerBlockCount.e(pageCounter);
                    rowsInPage.clear();
                    rowsInPage.reserve(maxRowsPerBlock);
                }
            }
        }
        // getline(fin, word);
    }
    if (!rowsInPage.empty())
    {
        bufferManager.writePage(this->MatrixName, /*page number = */ this->blockCount, /*data = */ rowsInPage, rowsInPage.size());
        this->blockCount++;
        rowsInPage.clear();
        // this->rowsPerBlockCount.emplace_back(pageCounter);
        // pageCounter = 0;
    }

    if (this->rowCount == 0)
        return false;
    // this->distinctValuesInColumns.clear();
    return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    this->sub_matrix_side = sqrt((1000 * BLOCK_SIZE) / sizeof(int));
    ifstream fin(this->sourceFileName, ios::in);

    string word;
    int page_number = 0;

    for (int row = 0; row < this->rowCount; ++row)
    {

        vector<int> page_data;
        int prev_page_number = page_number;
        for (int col = 0; col < this->columnCount; ++col)
        {
            if (col == this->columnCount - 1)
                getline(fin, word);
            else
                getline(fin, word, ',');

            page_data.push_back(stoi(word));

            if (((col + 1) % sub_matrix_side) == 0)
            {
                bufferManager.AppendPage(this->MatrixName, page_number, page_data);
                page_data.clear();
                page_number++;
            }
        }
        if (!page_data.empty())
        {
            bufferManager.AppendPage(this->MatrixName, page_number, page_data);
            page_number++;
        }
        if (((row + 1) % this->sub_matrix_side) != 0)
            page_number = prev_page_number;
    }
    this->blockCount = cel(this->rowCount, this->sub_matrix_side) * cel(this->columnCount, this->sub_matrix_side);
    if (this->rowCount == 0)
        return false;
    return true;
}

// /**
//  * @brief Given a row of values, this function will update the statistics it
//  * stores i.e. it updates the number of rows that are present in the column and
//  * the number of distinct values present in each column. These statistics are to
//  * be used during optimisation.
//  *
//  * @param row
//  */
// // void Matrix::updateStatistics(vector<int> row)
// // {
// //     this->rowCount++;
// //     for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
// //     {
// //         if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
// //         {
// //             this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
// //             this->distinctValuesPerColumnCount[columnCounter]++;
// //         }
// //     }
// // }
// /**
//  * @brief Checks if the given column is present in this Matrix.
//  *
//  * @param columnName
//  * @return true
//  * @return false
//  */
// bool Matrix::isColumn(string columnName)
// {
//     logger.log("Matrix::isColumn");
//     for (auto col : this->columns)
//     {
//         if (col == columnName)
//         {
//             return true;
//         }
//     }
//     return false;
// }

// /**
//  * @brief Renames the column indicated by fromColumnName to toColumnName. It is
//  * assumed that checks such as the existence of fromColumnName and the non prior
//  * existence of toColumnName are done.
//  *
//  * @param fromColumnName
//  * @param toColumnName
//  */
// void Matrix::renameColumn(string fromColumnName, string toColumnName)
// {
//     logger.log("Matrix::renameColumn");
//     for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
//     {
//         if (columns[columnCounter] == fromColumnName)
//         {
//             columns[columnCounter] = toColumnName;
//             break;
//         }
//     }
//     return;
// }

// /**
//  * @brief Function prints the first few rows of the Matrix. If the Matrix contains
//  * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
//  * the rows are printed.
//  *
//  */
// void Matrix::print()
// {
//     logger.log("Matrix::print");
//     uint count = min((long long)PRINT_COUNT, this->rowCount);

//     //print headings
//     this->writeRow(this->columns, cout);

//     Cursor cursor(this->MatrixName, 0);
//     vector<int> row;
//     for (int rowCounter = 0; rowCounter < count; rowCounter++)
//     {
//         row = cursor.getNext();
//         this->writeRow(row, cout);
// //     }
// //     printRowCount(this->rowCount);
// // }

// /**
//  * @brief This function returns one row of the Matrix using the cursor object. It
//  * returns an empty row is all rows have been read.
//  *
//  * @param cursor
//  * @return vector<int>
//  */
// void Matrix::getNextPage(Cursor *cursor)
// {
//     logger.log("Matrix::getNext");

//     if (cursor->pageIndex < this->blockCount - 1)
//     {
//         cursor->nextPage(cursor->pageIndex + 1);
//     }
// }

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    if (!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->MatrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    //print headings
    // this->writeRow(this->columns, fout);

    // Cursor cursor(this->MatrixName, 0);

    if (is_sparse)
    {
        Page *cur_page = bufferManager.getPage(this->MatrixName, 0);
        int cur_page_number = 0, page_pointer = 0;
        int row_index, col_index;
        if (this->columns[0] == "rows")
        {
            row_index = 0;
            col_index = 1;
            bool blocks_finished = false;
            for (int i = 0; i < this->rowCount; ++i)
            {

                for (int j = 0; j < this->columnCount; ++j)
                {
                    if (blocks_finished)
                    {
                        fout << 0;
                        fout << ((j == this->columnCount - 1) ? "\n" : ",");
                        continue;
                    }
                    if (page_pointer == cur_page->rows.size())
                    {
                        cur_page_number++;
                        if (cur_page_number == this->blockCount)
                        {
                            blocks_finished = true;
                            fout << 0;
                            fout << ((j == this->columnCount - 1) ? "\n" : ",");
                            continue;
                        }
                        else
                        {
                            cur_page = bufferManager.getPage(this->MatrixName, cur_page_number);
                            page_pointer = 0;
                        }
                    }
                    if (cur_page->rows[page_pointer][row_index] == i && cur_page->rows[page_pointer][col_index] == j)
                    {
                        fout << cur_page->rows[page_pointer][2];
                        fout << ((j == this->columnCount - 1) ? "\n" : ",");
                        page_pointer++;
                    }
                    else
                    {
                        fout << 0;
                        fout << ((j == this->columnCount - 1) ? "\n" : ",");
                    }
                }
            }
        }
        else
        {
            row_index = 1;
            col_index = 0;
            for (int i = 0; i < this->rowCount; ++i)
            {
                for (int j = 0; j < this->columnCount; ++j)
                {
                    fout << FindInCompressedTable(i, j);
                    fout << ((j == this->columnCount - 1) ? "\n" : ",");
                }
            }
        }
    }
    else
    {

        for (int rowCounter = 0; rowCounter < this->rowCount; ++rowCounter)
        {
            int cur_page_number = GetPageNumberFromCoordinates(rowCounter, 0);
            Page *cur_page = bufferManager.getPage(this->MatrixName, cur_page_number);
            int relative_x = rowCounter % this->sub_matrix_side;
            int expected_page_number;
            for (int colCounter = 0; colCounter < this->columnCount; ++colCounter)
            {

                if (colCounter % this->sub_matrix_side == 0)
                    expected_page_number = GetPageNumberFromCoordinates(rowCounter, colCounter);
                int relative_y = colCounter % this->sub_matrix_side;

                if (cur_page_number != expected_page_number)
                {
                    cur_page_number = expected_page_number;
                    cur_page = bufferManager.getPage(this->MatrixName, cur_page_number);
                }

                fout << cur_page->rows[relative_x][relative_y];
                fout << ((colCounter == this->columnCount - 1) ? "\n" : ",");
            }
        }
    }

    fout.close();
}

/**
 * @brief Function to check if Matrix is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->MatrixName + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the Matrix from the database by deleting
 * all temporary files created as part of this Matrix
 *
 */
void Matrix::unload()
{
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->MatrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this Matrix
 * 
 * @return Cursor 
 */
Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->MatrixName, 0);
    return cursor;
}

int Matrix::GetPageNumberFromCoordinates(int row, int col)
{
    int pages_in_one_row = cel(this->rowCount, this->sub_matrix_side);
    return (row / this->sub_matrix_side) * pages_in_one_row + col / this->sub_matrix_side;
}

int Matrix::FindInCompressedTable(int row, int col)
{

    int l = 0, f = this->blockCount - 1;
    while (l <= f)
    {

        int mid = l + (f - l) / 2;
        Page *p = bufferManager.getPage(this->MatrixName, mid);

        int frst = p->rows[0][0];
        int last = p->rows.back().at(0);
        if (frst > col)
        {
            f = mid - 1;
        }
        else if (last < col)
        {
            l = mid + 1;
        }
        else
        {
            if (frst != last)
            {
                // page found, apply search on this page.
                return FindInPage(p, row, col);
            }
            else
            {
                // could be other page too
                if (p->rows[0][1] == row)
                    return p->rows[0][2];
                else if (p->rows[0][1] > row)
                {
                    f = mid - 1;
                    continue;
                }
                else if (p->rows.back().at(1) == row)
                {
                    return p->rows.back().at(2);
                }
                else if (p->rows.back().at(1) < row)
                {
                    l = mid + 1;
                    continue;
                }
                else
                    return FindInPage(p, row, col);
            }
        }
    }
    // not found
    return 0;
}
void Matrix::print()
{

    logger.log("inside print matrix function");

    if (is_sparse)
    {
        Page *cur_page = bufferManager.getPage(this->MatrixName, 0);
        int cur_page_number = 0, page_pointer = 0;
        int row_index, col_index;
        if (this->columns[0] == "rows")
        {
            row_index = 0;
            col_index = 1;
            bool blocks_finished = false;
            for (int i = 0; i < min(20, (int)this->rowCount); ++i)
            {
                for (int j = 0; j < this->columnCount; ++j)
                {
                    if (blocks_finished)
                    {
                        cout << 0 << " ";
                        continue;
                    }
                    if (page_pointer == cur_page->rows.size())
                    {
                        cur_page_number++;
                        if (cur_page_number == this->blockCount)
                        {
                            blocks_finished = true;
                            cout << 0 << " ";
                            continue;
                        }
                        else
                        {
                            cur_page = bufferManager.getPage(this->MatrixName, cur_page_number);
                            page_pointer = 0;
                        }
                    }
                    if (cur_page->rows[page_pointer][row_index] == i && cur_page->rows[page_pointer][col_index] == j)
                    {
                        cout << cur_page->rows[page_pointer][2] << " ";
                        page_pointer++;
                    }
                    else
                        cout << 0 << " ";
                }
                cout << endl;
            }
        }
        else
        {
            row_index = 1;
            col_index = 0;
            for (int i = 0; i < this->rowCount; ++i)
            {

                for (int j = 0; j < this->columnCount; ++j)
                {
                    cout << FindInCompressedTable(i, j) << " ";
                }
                cout << endl;
            }
        }
    }
    else
    {
        Page *cur_page = bufferManager.getPage(this->MatrixName, 0);
        int cur_page_number = 0;

        for (int row = 0; row < min(20, (int)this->rowCount); ++row)
        {
            for (int col = 0; col < this->columnCount; ++col)
            {
                int expected_page_number = GetPageNumberFromCoordinates(row, col);

                if (cur_page_number != expected_page_number)
                {
                    cur_page_number = expected_page_number;
                    cur_page = bufferManager.getPage(this->MatrixName, cur_page_number);
                }
                int relative_x = row % this->sub_matrix_side;
                int relative_y = col % this->sub_matrix_side;
                // cout << relative_y << "," << relative_y << " ";
                cout << cur_page->rows[relative_x][relative_y] << " ";
            }
            cout << "\n";
        }
    }
}
void Matrix::transpose()
{

    logger.log("inside the transpose function");
    if (this->is_sparse)
    {
        swap(this->columns[0], this->columns[1]);
        // now sort it according to row and then according to column
    }
    else
    {
        int blocks_in_row = cel(this->columnCount, this->sub_matrix_side);
        for (int i = 0; i < blocks_in_row; ++i)
        {
            for (int j = i; j < blocks_in_row; ++j)
            {
                int page_number_1 = blocks_in_row * i + j;
                int page_number_2 = blocks_in_row * j + i;
                Page *page_1 = bufferManager.getPage(this->MatrixName, page_number_1);
                Page *page_2 = bufferManager.getPage(this->MatrixName, page_number_2);
                SwapPages(page_1, page_2);
                page_1->writePage();
                page_2->writePage();
            }
        }
    }
    swap(this->rowCount, this->columnCount);
}
// /**
//  * @brief Function that returns the index of column indicated by columnName
//  *
//  * @param columnName
//  * @return int
//  */
// int Matrix::getColumnIndex(string columnName)
// {
//     logger.log("Matrix::getColumnIndex");
//     for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
//     {
//         if (this->columns[columnCounter] == columnName)
//             return columnCounter;
//     }
// }
