
#ifndef _INCL_BUFFERMANAGER_H
#define _INCL_BUFFERMANAGER_H

#include "page.h"
/**
 * @brief The BufferManager is responsible for reading pages to the main memory.
 * Recall that large files are broken and stored as blocks in the hard disk. The
 * minimum amount of memory that can be read from the disk is a block whose size
 * is indicated by BLOCK_SIZE. within this system we simulate blocks by
 * splitting and storing the file as multiple files each of one BLOCK_SIZE,
 * although this isn't traditionally how it's done. You can alternatively just
 * random access to the point where a block begins within the same
 * file. In this system we assume that the the sizes of blocks and pages are the
 * same. 
 * 
 * <p>
 * The buffer can hold multiple pages quantified by BLOCK_COUNT. The
 * buffer manager follows the FIFO replacement policy i.e. the first block to be
 * read in is replaced by the new incoming block. This replacement policy should
 * be transparent to the executors i.e. the executor should not know if a block
 * was previously present in the buffer or was read in from the disk. 
 * </p>
 *
 */
class BufferManager
{

    deque<Page *> pages;
    deque<Page *> pages_having_one_row;
    bool inPool(const string &pageName);
    bool inPool2(const string &pageName, int rowNumber);
    Page *getFromPool(const string &pageName);
    Page *getFromPool2(const string &pageName, int rowNumber);
    Page *insertIntoPool(const string &tableName, int pageIndex, bool is_matrix);
    Page *insertIntoPool2(const string &tableName, int pageIndex, int rowNumber);

public:
    BufferManager();
    Page *getPage(const string &tableName, int pageIndex);
    Page *getPage(const string &tableName, int pageIndex, int pageRow);
    // void writePage(string pageName, vector<vector<int>> rows);
    void deleteFile(string tableName, int pageIndex);
    void deleteFile(string fileName);
    void writePage(const string &tableName, int pageIndex, const vector<vector<int>> &rows, int rowCount);
    void AppendPage(const string &matrix_name, int page_index, const vector<int> &row);
    void PopPool();
};

#endif
