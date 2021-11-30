#include "../global.h"

using Record = vector<int>;
namespace
{
    Record *GetRecord(ifstream *fp)
    {

        Record *ans = new vector<int>;
        string line = "";
        if (!getline(*fp, line) || line == "")
        {
            return ans;
        }
        stringstream fp2(line);
        string word;
        while (getline(fp2, word, ' '))
        {
            ans->push_back(stoi(word));
        }
        return ans;
    }
    template <typename T>
    void PushIntoFile(ofstream &fp, T *ptr)
    {
        int itr = 0;
        for (auto value : *ptr)
        {
            fp << value;
            if (itr++ != ptr->size() - 1)
                fp << ",";
        }
        fp << endl;
    }
}

namespace SortTable
{

    void sort(Table *table, int idx, int mode /*acs = 1, desc = 0*/, const string &final_table_name)
    {
        vector<ifstream *> opened_files;
        int file_count = 0;
        for (int i = 0; i < table->blockCount;)
        {

            vector<Record> data;
            for (int c = 0; c < BUFFER_SIZE && i < table->blockCount; c++)
            {
                Page *p = bufferManager.getPage(table->tableName, i);
                int cnt = 0;
                for (auto &rcd : p->rows)
                {
                    if (cnt++ == p->rowCount)
                        break;
                    data.push_back(rcd);
                }
                i++;
            }
            if (mode == 1)
                sort(data.begin(), data.end(), [&idx](Record &record1, Record &record2)
                     { return record1[idx] < record2[idx]; });
            else
                sort(data.begin(), data.end(), [&idx](Record &record1, Record &record2)
                     { return record1[idx] > record2[idx]; });

            string file_name = "../data/temp/" + to_string(++file_count) + ".txt";
            ofstream fout(file_name, ios::out);
            for (auto &record : data)
            {
                int it = 0;
                for (auto value : record)
                {
                    fout << value;
                    if (it++ != record.size() - 1)
                        fout << " ";
                }
                fout << endl;
            }
            fout.close();
            ifstream *file_pointer = new ifstream(file_name.c_str());
            opened_files.push_back(file_pointer);
        }
        using T = pair<int, pair<int, Record *>>;
        auto comp = [&mode](T &A, T &B)
        { return (A < B) ^ mode; };
        priority_queue<T, vector<T>, decltype(comp)> que(comp);
        string filename = "../data/temp/sorted_output.csv";
        ofstream final_file(filename, ios::out);
        PushIntoFile(final_file, &table->columns);
        int cnt = 0;
        for (auto fp : opened_files)
        {
            Record *new_record = GetRecord(fp);
            if (!new_record->empty())
                que.push({new_record->at(idx), {cnt++, new_record}});
        }
        while (!que.empty())
        {
            auto [value, temp] = que.top();
            auto [idx_file, record_ptr] = temp;
            PushIntoFile(final_file, record_ptr);
            record_ptr = GetRecord(opened_files[idx_file]);
            que.pop();
            if (!record_ptr->empty())
                que.push({record_ptr->at(idx), {idx_file, record_ptr}});
        }
        final_file.close();

        Table *final_table = new Table();
        final_table->sourceFileName = filename;
        final_table->tableName = final_table_name;
        final_table->load();
        tableCatalogue.insertTable(final_table);
        for (int i = 0; i < file_count; i++)
        {
            remove(("../data/temp/" + to_string(i + 1) + ".txt").c_str());
        }
        remove(filename.c_str());
    }
}

/**
 * @brief File contains method to process SORT commands.
 *
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 *
 * sorting_order = ASC | DESC
 */

bool is_number(const std::string &s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}
bool syntacticParseSORT()
{
    logger.log("syntacticParseSORT");
    if (tokenizedQuery.size() != 8 && tokenizedQuery.size() != 10)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (tokenizedQuery[4] != "BY" || tokenizedQuery[6] != "IN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = SORT;
    parsedQuery.sortResultRelationName = tokenizedQuery[0];
    parsedQuery.sortRelationName = tokenizedQuery[3];
    parsedQuery.sortColumnName = tokenizedQuery[5];
    string sortingStrategy = tokenizedQuery[7];
    if (sortingStrategy == "ASC")
        parsedQuery.sortingStrategy = ASC;
    else if (sortingStrategy == "DESC")
        parsedQuery.sortingStrategy = DESC;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (tokenizedQuery.size() == 10)
    {

        if (tokenizedQuery[8] != "BUFFER" || !is_number(tokenizedQuery[9]))
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        BUFFER_SIZE = stoi(tokenizedQuery[9]);
        if (BUFFER_SIZE <= 0)
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        return true;
    }
}

bool semanticParseSORT()
{
    logger.log("semanticParseSORT");

    if (tableCatalogue.isTable(parsedQuery.sortResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnName, parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    return true;
}

void executeSORT()
{
    logger.log("executeSORT");
    Table *table = tableCatalogue.getTable(parsedQuery.sortRelationName);
    int mode = parsedQuery.sortingStrategy == ASC;
    int col_idx = table->getColumnIndex(parsedQuery.sortColumnName);
    SortTable::sort(table, col_idx, mode, parsedQuery.sortResultRelationName);
    BUFFER_SIZE = 10;

    return;
}