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
    bool InsertRecordIntoPage(Page &p, const vector<int> &record1, const vector<int> &record2, int &page_index, Table *final_table)
    {
        vector<int> record = record1;
        for (auto it : record2)
            record.push_back(it);
        p.rows.push_back(record);
        if (p.rows.size() == BLOCK_SIZE)
        {
            final_table->rowCount += p.rows.size();
            p.rowCount = p.rows.size();
            p.columnCount = p.rows[0].size();
            p.pageName = "../data/temp/" + p.tableName + "_Page" + to_string(page_index);
            p.writePage();
            p.rows.clear();
            page_index++;
            p.rowCount = 0;
            p.columnCount = 0;
            final_table->blockCount++;
            final_table->rowsPerBlockCount.push_back(BLOCK_SIZE);
            return true;
        }
        return false;
    }
    using Record = vector<int>;
    int Join(string name1, string name2, string column1, string column2, string result_relation, Table *final_table, int &page_index)
    {

        Page result = Page();
        result.tableName = result_relation;

        unordered_map<int, vector<Record>> outer;
        int idx = 0;

        Table *A = tableCatalogue.getTable(name1);
        Table *B = tableCatalogue.getTable(name2);
        // TODO
        //  if (A->rowCount > B->rowCount)
        //  {
        //      swap(name1, name2);
        //      swap(column1, column2);
        //      swap(A, B);
        //  }
        int idx1 = A->getColumnIndex(column1);
        int idx2 = B->getColumnIndex(column2);

        int block_accesses = 0;
        while (1)
        {

            for (int i = 0; i < min(nB - 2, A->blockCount - idx); i++)
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
                block_accesses++;
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
                        if (InsertRecordIntoPage(result, record1, record2, page_index, final_table))
                            block_accesses++;
                    }
                }
            }
            outer.clear();
        }
        if (!result.rows.empty())
        {

            // cout << "inserted record";
            final_table->rowsPerBlockCount.push_back(result.rows.size());
            final_table->rowCount += result.rows.size();
            result.rowCount = result.rows.size();
            result.columnCount = result.rows[0].size();
            result.pageName = "../data/temp/" + result.tableName + "_Page" + to_string(page_index);
            result.writePage();
            block_accesses++;
            result.rows.clear();
            final_table->blockCount++;
            page_index++;
        }

        // cout << "Done block nested join\n No. of block accesses  = " << block_accesses << endl;
        return block_accesses;
    }

    const int M = nB - 1;

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
        return true;
    }

    bool InsertRecordIntoPage2(Page *p, vector<int> &record, const string &filename)
    {
        p->rows.push_back(record);
        if (p->rows.size() == BLOCK_SIZE)
        {
            return clear_buffer(p, filename);
        }
        return false;
    }
    int Hash(int r)
    {
        return r % M;
    }

    int partitionJoin(string name1, string name2, string column1, string column2, string result_relation, Table *final_table)
    {
        Table *A = tableCatalogue.getTable(name1);
        Table *B = tableCatalogue.getTable(name2);
        int idx1 = A->getColumnIndex(column1);
        int idx2 = B->getColumnIndex(column2);

        vector<Page *> buffers(M);
        vector<string> filenameA(M), filenameB(M);
        int block_accesses = 0;
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
            block_accesses++;
            int cnt = 0;
            for (auto record : p->rows)
            {
                if (cnt++ == p->rowCount)
                    break;

                int bucket_number = Hash(record[idx1]);
                if (InsertRecordIntoPage2(buffers[bucket_number], record, filenameA[bucket_number]))
                    block_accesses++;
            }
        }
        for (int i = 0; i < M; i++)
        {
            if (clear_buffer(buffers[i], filenameA[i]))
                block_accesses++;
        }

        for (int i = 0; i < B->blockCount; i++)
        {
            Page *p = bufferManager.getPage(name2, i);
            block_accesses++;
            int cnt = 0;
            for (auto record : p->rows)
            {
                if (cnt++ == p->rowCount)
                    break;

                int bucket_number = Hash(record[idx2]);
                if (InsertRecordIntoPage2(buffers[bucket_number], record, filenameB[bucket_number]))
                    block_accesses++;
            }
        }
        for (int i = 0; i < M; i++)
        {
            if (clear_buffer(buffers[i], filenameB[i]))
                block_accesses++;
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
                continue;
            if (A->rowCount == 0 or B->rowCount == 0)
            {
                // cout << A->rowCount << " " << B->rowCount << "i = ";
                // cout << i << endl;
                continue;
            }
            // cout << a << " " << b << endl;
            // continue;

            block_accesses += Join(a, b, column1, column2, result_relation, final_table, page_index);
        }
        return block_accesses;
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
    if (ch == 0)
    {
        int page_index = 0;
        int block_accesses = Join(parsedQuery.joinFirstRelationName, parsedQuery.joinSecondRelationName, parsedQuery.joinFirstColumnName, parsedQuery.joinSecondColumnName, parsedQuery.joinResultRelationName, final_table, page_index);
        cout << "Done block nested join\n No. of block accesses  = " << block_accesses << endl;
    }
    else
    {
        int block_accesses = partitionJoin(parsedQuery.joinFirstRelationName, parsedQuery.joinSecondRelationName, parsedQuery.joinFirstColumnName, parsedQuery.joinSecondColumnName, parsedQuery.joinResultRelationName, final_table);
        cout << "Done partition join\n No. of block accesses  = " << block_accesses << endl;
    }

    logger.log("executeJOIN");
    return;
}