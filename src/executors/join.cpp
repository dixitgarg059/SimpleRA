#include "../global.h"
/**
 * @brief
 * SYNTAX: R <- JOIN Using type relation_name1, relation_name2 ON column_name1 bin_op column_name2 BUFFER num
 */

namespace
{
    bool is_number(const std::string &s)
    {
        return !s.empty() && std::find_if(s.begin(),
                                          s.end(), [](unsigned char c)
                                          { return !std::isdigit(c); }) == s.end();
    }
    bool check_syntax(const vector<string> &tq)
    {
        if (tq.size() != 13)
            return false;
        vector<string> syntax_vector_actual = {"<-", "JOIN", "USING", "ON", "==", "BUFFER"};
        vector<string> syntax_vector_got = {tq[1], tq[2], tq[3], tq[7], tq[9], tq[11]};
        return syntax_vector_actual == syntax_vector_got and is_number(tq.back());
    }

}
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (!check_syntax(tokenizedQuery))
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    auto &tq = tokenizedQuery;

    if (tq[4] == "NESTED")
        parsedQuery.queryType = JOIN_BLOCK_NESTED;
    else if (tq[4] == "PARTHASH")
        parsedQuery.queryType = JOIN_PART_HASH;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.joinResultRelationName = tq[0];
    parsedQuery.joinFirstRelationName = tq[5];
    parsedQuery.joinSecondRelationName = tq[6];
    parsedQuery.joinFirstColumnName = tq[8];
    parsedQuery.joinSecondColumnName = tq[10];
    parsedQuery.joinBinaryOperator = EQUAL;
    cout << nB;
    nB = stoi(tq.back());
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

#define min(a, b) (a > b ? b : a)
namespace
{

    bool swaped = false;
    bool InsertRecordIntoPage(Page &p, vector<int> record1, vector<int> record2, int &page_index, Table *final_table)
    {

        if (swaped)
            std::swap(record1, record2);
        vector<int> record = record1;
        for (auto it : record2)
            record.push_back(it);
        p.rows.push_back(record);

        if (!p.rows.empty() and !p.rows[0].empty() and p.rows.size() >= ((int)BLOCK_SIZE * 1000 / (sizeof(int) * p.rows[0].size())))
        {

            final_table->rowCount += p.rows.size();
            p.rowCount = p.rows.size();
            p.columnCount = p.rows[0].size();
            p.pageName = "../data/temp/" + p.tableName + "_Page" + to_string(page_index);
            p.writePage();
            final_table->rowsPerBlockCount.push_back(p.rows.size());
            p.rows.clear();
            page_index++;
            p.rowCount = 0;
            p.columnCount = 0;
            final_table->blockCount++;
            return true;
        }
        return false;
    }
    using Record = vector<int>;
    void Join(string name1, string name2, string column1, string column2, string result_relation, Table *final_table, int &page_index, Page &result, bool not_called_by_partition_hash = true)
    {

        unordered_map<int, vector<Record>> outer;
        int idx = 0;

        Table *A = tableCatalogue.getTable(name1);
        Table *B = tableCatalogue.getTable(name2);
        swaped = false;
        if (A->rowCount > B->rowCount)
        {
            swap(name1, name2);
            swap(column1, column2);
            swap(A, B);
            swaped = true;
        }
        int idx1 = A->getColumnIndex(column1);
        int idx2 = B->getColumnIndex(column2);

        while (1)
        {

            int idx_temp = idx;
            for (int i = 0; i < min(nB - 2, A->blockCount - idx_temp); i++)
            {
                Page *p = bufferManager.getPage(name1, idx);
                int cnt = 0;
                for (const auto &record : p->rows)
                {
                    if (cnt++ == p->rowCount)
                        break;
                    outer[record[idx1]].push_back(record);
                }
                idx++;
            }
            if (outer.empty())
                break;
            for (int i = 0; i < B->blockCount; i++)
            {
                Page *p = bufferManager.getPage(name2, i);
                int cnt1 = 0;
                for (auto &record2 : p->rows)
                {
                    if (cnt1++ == p->rowCount)
                        break;

                    if (record2.size() <= idx2)
                        continue;
                    int val2 = record2[idx2];

                    for (auto record1 : outer[val2])
                    {
                        InsertRecordIntoPage(result, record1, record2, page_index, final_table);
                    }
                }
            }
            outer.clear();
        }
        if (not_called_by_partition_hash && !result.rows.empty())
        {

            final_table->rowsPerBlockCount.push_back(result.rows.size());
            final_table->rowCount += result.rows.size();
            result.rowCount = result.rows.size();
            result.columnCount = result.rows[0].size();
            result.pageName = "../data/temp/" + result.tableName + "_Page" + to_string(page_index);
            result.writePage();
            result.rows.clear();
            final_table->blockCount++;
        }
        return;
    }

    int M = nB - 1;

    bool clear_buffer(Page *p, const string &filename)
    {

        if (p->rows.empty())
            return false;
        ofstream fout(filename, ios::app);
        for (auto row : p->rows)
        {
            for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
            {
                if (columnCounter != 0)
                    fout << ",";
                fout << row[columnCounter];
            }
            fout << endl;
        }
        fout.close();
        p->rows.clear();
        block_accesses_write++;
        return true;
    }

    bool InsertRecordIntoPage2(Page *p, vector<int> &record, const string &filename)
    {
        p->rows.push_back(record);
        if (!p->rows.empty() and !p->rows[0].empty() and p->rows.size() >= ((int)BLOCK_SIZE * 1000 / (sizeof(int) * p->rows[0].size())))
        {
            return clear_buffer(p, filename);
        }
        return false;
    }
    int Hash(int r)
    {

        long long int x = r;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return (int)(x % M);
    }

    void partitionJoin(string name1, string name2, string column1, string column2, string result_relation, Table *final_table, Page &result)
    {
        Table *A = tableCatalogue.getTable(name1);
        Table *B = tableCatalogue.getTable(name2);
        int idx1 = A->getColumnIndex(column1);
        int idx2 = B->getColumnIndex(column2);

        vector<Page *> buffers(M);
        vector<string> filenameA(M), filenameB(M);

        for (int i = 0; i < M; i++)
        {
            filenameA[i] = "../data/Partition_A" + to_string(i) + ".csv";
            filenameB[i] = "../data/Partition_B" + to_string(i) + ".csv";
            ofstream fout(filenameA[i], ios::out);
            for (const auto &col : A->columns)
            {
                fout << col;
                if (col != A->columns.back())
                    fout << ",";
            }
            fout << endl;
            fout.close();

            ofstream fout2(filenameB[i], ios::out);
            for (const auto &col : B->columns)
            {
                fout2 << col;
                if (col != B->columns.back())
                    fout2 << ",";
            }
            fout2 << endl;
            fout2.close();
        }
        for (auto &it : buffers)
        {
            it = new Page();
        }
        for (int i = 0; i < A->blockCount; i++)
        {
            Page *p = bufferManager.getPage(name1, i);
            int cnt = 0;
            for (auto record : p->rows)
            {
                if (cnt++ == p->rowCount)
                    break;

                int bucket_number = Hash(record[idx1]);
                InsertRecordIntoPage2(buffers[bucket_number], record, filenameA[bucket_number]);
            }
        }
        for (int i = 0; i < M; i++)
        {
            clear_buffer(buffers[i], filenameA[i]);
        }

        for (int i = 0; i < B->blockCount; i++)
        {
            Page *p = bufferManager.getPage(name2, i);
            int cnt = 0;
            for (auto record : p->rows)
            {
                if (cnt++ == p->rowCount)
                    break;

                int bucket_number = Hash(record[idx2]);
                InsertRecordIntoPage2(buffers[bucket_number], record, filenameB[bucket_number]);
            }
        }
        for (int i = 0; i < M; i++)
        {
            clear_buffer(buffers[i], filenameB[i]);
        }

        int page_index = 0;
        for (int i = 0; i < M; i++)
        {

            string a = "Partition_A" + to_string(i);
            string b = "Partition_B" + to_string(i);
            Table *A = new Table(a);
            Table *B = new Table(b);
            if (A->load() && B->load())
            {
                tableCatalogue.insertTable(A);
                tableCatalogue.insertTable(B);
            }
            else
                goto Continue;
            if (A->rowCount == 0 or B->rowCount == 0)
                goto Continue;

            Join(a, b, column1, column2, result_relation, final_table, page_index, result, false);
        Continue:
            A->unload();
            B->unload();
            remove(A->sourceFileName.c_str());
            remove(B->sourceFileName.c_str());
        }
        if (!result.rows.empty())
        {

            // if (swaped)
            // std::swap(record1, record2);
            final_table->rowsPerBlockCount.push_back(result.rows.size());
            final_table->rowCount += result.rows.size();
            result.rowCount = result.rows.size();
            result.columnCount = result.rows[0].size();
            result.pageName = "../data/temp/" + result.tableName + "_Page" + to_string(page_index);
            result.writePage();
            result.rows.clear();
            final_table->blockCount++;
        }

        return;
    }
}
void executeJOIN(int ch)
{

    Table *A = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *B = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);
    vector<string> result_columns = A->columns;
    for (auto it : B->columns)
        result_columns.push_back(it);
    Table *final_table = new Table(parsedQuery.joinResultRelationName, result_columns);
    tableCatalogue.insertTable(final_table);
    Page result = Page();
    result.tableName = parsedQuery.joinResultRelationName;
    if (ch == 0)
    {
        int page_index = 0;
        block_accesses_read = 0;
        block_accesses_write = 0;
        Join(parsedQuery.joinFirstRelationName, parsedQuery.joinSecondRelationName, parsedQuery.joinFirstColumnName, parsedQuery.joinSecondColumnName, parsedQuery.joinResultRelationName, final_table, page_index, result);
        std::remove(final_table->sourceFileName.c_str());
        std::cout << "Done block nested join\n No. of block accesses  for reading = " << block_accesses_read << "\n No. of block accesses for writing = " << block_accesses_write << endl;
    }
    else
    {
        M = nB - 1;
        block_accesses_read = 0;
        block_accesses_write = 0;
        partitionJoin(parsedQuery.joinFirstRelationName, parsedQuery.joinSecondRelationName, parsedQuery.joinFirstColumnName, parsedQuery.joinSecondColumnName, parsedQuery.joinResultRelationName, final_table, result);
        std::remove(final_table->sourceFileName.c_str());
        std::cout << "Done partition hash join\n No. of block accesses  for reading = " << block_accesses_read << "\n No. of block accesses for writing = " << block_accesses_write << endl;
    }

    logger.log("executeJOIN");
    return;
}