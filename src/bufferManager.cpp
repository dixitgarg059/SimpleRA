#include "global.h"
BufferManager::BufferManager()
{
    logger.log("BufferManager::BufferManager");
}

/**
 * @brief Function called to read a page from the buffer manager. If the page is
 * not present in the pool, the page is read and then inserted into the pool.
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page *BufferManager::getPage(const string &tableName, int pageIndex)
{
    // cout << "hello ";
    // return;
    // logger.log("BufferManager::getPage");
    string pageName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    if (this->inPool(pageName))
    {
        // cout << "1";
        return this->getFromPool(pageName);
    }
    else
    {
        // cout << "2";
        bool is_matrix = true;
        if (tableCatalogue.getTable(tableName))
            is_matrix = false;
        return this->insertIntoPool(tableName, pageIndex, is_matrix);
    }
}

Page *BufferManager::getPage(const string &tableName, int pageIndex, int pageRow)
{
    string pageName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    if (this->inPool2(pageName, pageRow))
    {
        // cout << "1";
        return this->getFromPool2(pageName, pageRow);
    }
    else
    {
        return this->insertIntoPool2(tableName, pageIndex, pageRow);
    }
}

/**
 * @brief Checks to see if a page exists in the pool
 *
 * @param pageName 
 * @return true 
 * @return false 
 */
bool BufferManager::inPool(const string &pageName)
{
    // logger.log("BufferManager::inPool");
    for (auto page : this->pages)
    {
        if (pageName == page->pageName)
            return true;
    }
    return false;
}

bool BufferManager::inPool2(const string &pageName, int rowNumber)
{
    // logger.log("BufferManager::inPool");
    for (auto page : this->pages_having_one_row)
    {
        if (pageName == page->pageName && rowNumber == page->cur_row)
            return true;
    }
    return false;
}

/**
 * 
 * 
 * @brief If the page is present in the pool, then this function returns the
 * page. Note that this function will fail if the page is not present in the
 * pool.
 *
 * @param pageName 
 * @return Page 
 */
Page *BufferManager::getFromPool(const string &pageName)
{
    // logger.log("BufferManager::getFromPool");
    for (auto page : this->pages)
        if (pageName == page->pageName)
            return page;
}

Page *BufferManager::getFromPool2(const string &pageName, int rowNumber)
{
    // logger.log("BufferManager::getFromPool");
    for (auto page : this->pages_having_one_row)
        if (pageName == page->pageName && rowNumber == page->cur_row)
            return page;
}

/**
 * @brief Inserts page indicated by tableName and pageIndex into pool. If the
 * pool is full, the pool ejects the oldest inserted page from the pool and adds
 * the current page at the end. It naturally follows a queue data structure. 
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page *BufferManager::insertIntoPool(const string &tableName, int pageIndex, bool is_matrix)
{

    while (!pages_having_one_row.empty())
    {
        delete pages_having_one_row.front();
        pages_having_one_row.pop_front();
    }
    if (is_matrix)
    {
        // logger.log("BufferManager::insertIntoPool for matrix");
        string file_name = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
        ifstream fin(file_name, ios::in);
        string line;
        // Page page;
        vector<vector<int>> data;
        vector<int> row;
        if (this->pages.size() >= BLOCK_COUNT)
        {
            delete pages.front();
            pages.pop_front();
        }
        Page *page = new Page(tableName, pageIndex, data, data.size());
        while (getline(fin, line))
        {

            string word;
            stringstream s(line);
            int no_columns = 0;
            page->rows.push_back(row);
            if (page->rows.size() > 1)
                page->rows.back().reserve(prev(page->rows.end())->size());
            while (getline(s, word, ' '))
            {
                no_columns++;
                page->rows[page->rowCount].push_back(stoi(word));
            }
            page->columnCount = no_columns;
            page->rowCount++;
        }
        pages.push_back(page);
        return page;
    }
    else
    {

        // logger.log("BufferManager::insertIntoPool for table");
        if (this->pages.size() >= BLOCK_COUNT)
        {
            delete pages.front();
            pages.pop_front();
        }
        Page *page = new Page(tableName, pageIndex);
        pages.push_back(page);
        return page;
    }
}

Page *BufferManager::insertIntoPool2(const string &tableName, int pageIndex, int rowNumber)
{

    // logger.log("BufferManager::insertIntoPool for matrix");
    pages.clear();
    string file_name = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    ifstream fin(file_name, ios::in);
    string line;
    // Page page;
    Page *page = new Page();
    page->tableName = tableName;
    page->rowCount = 1;
    page->pageIndex = pageIndex;
    if (this->pages_having_one_row.size() >= BLOCK_COUNT)
    {
        delete pages_having_one_row.front();
        pages_having_one_row.pop_front();
    }
    int rowCounter = 0;
    while (getline(fin, line))
    {

        if (rowCounter == rowNumber)
        {
            string word;
            stringstream s(line);
            while (getline(s, word, ' '))
            {
                page->columnCount++;
                page->row.push_back(stoi(word));
            }
            break;
        }
        else
        {
            rowCounter++;
        }
    }
    pages_having_one_row.push_back(page);
    return page;
}
/**
 * @brief The buffer manager is also responsible for writing pages. This is
 * called when new tables are created using assignment statements.
 *
 * @param tableName 
 * @param pageIndex 
 * @param rows 
 * @param rowCount 
 */
void BufferManager::writePage(const string &tableName, int pageIndex, const vector<vector<int>> &rows, int rowCount)
{
    // logger.log("BufferManager::writePage");
    string pageName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    ofstream fout(pageName, ios::trunc);
    for (const auto &row : rows)
    {
        for (const auto &col : row)
            fout << col << " ";
        fout << "\n";
    }
    fout.close();
}

/**
 * @brief Appends a row in the page, used in load matrix operation
 * 
 * */
void BufferManager::AppendPage(const string &table_name, int page_index, const vector<int> &row)
{
    logger.log("BufferManager::AppendPage");
    string file_name = "../data/temp/" + table_name + "_Page" + to_string(page_index);
    ofstream fout(file_name, ios::app);
    for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
    {
        if (columnCounter != 0)
            fout << " ";
        fout << row[columnCounter];
    }
    fout << endl;
    fout.close();
}
/**
 * @brief Deletes file names fileName
 *
 * @param fileName 
 */
void BufferManager::deleteFile(string fileName)
{

    if (remove(fileName.c_str()))
        logger.log("BufferManager::deleteFile: Err");
    else
        logger.log("BufferManager::deleteFile: Success");
}

/**
 * @brief Overloaded function that calls deleteFile(fileName) by constructing
 * the fileName from the tableName and pageIndex.
 *
 * @param tableName 
 * @param pageIndex 
 */
void BufferManager::deleteFile(string tableName, int pageIndex)
{
    logger.log("BufferManager::deleteFile");
    string fileName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    this->deleteFile(fileName);
}