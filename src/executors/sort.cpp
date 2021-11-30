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
    bool InsertRecordIntoPage(Page &p, vector<int> record, int &page_index, int iteration, int run_number)
    {

        p.rows.push_back(record);
        p.rowCount = p.rows.size();
        p.columnCount = p.rows[0].size();
        p.pageName = "../data/temp/" + to_string(iteration) + "_run_" + to_string(run_number) + "_Page" + to_string(page_index);

        if (!p.rows.empty() and !p.rows[0].empty() and p.rows.size() >= (int)((BLOCK_SIZE * 1000) / (sizeof(int) * p.rows[0].size())))
        {

            p.writePage();
            p.rows.clear();
            page_index++;
            p.rowCount = 0;
            p.columnCount = 0;
            return true;
        }
        return false;
    }

    Page getPage(string path, int page_number)
    {
        Page p = Page();
        path = "../data/temp/" + path + "_Page" + to_string(page_number);
        p.pageName = path;
        ifstream fin(path, ios::in);
        if (!fin.good())
            return p;
        string line;
        while (getline(fin, line))
        {
            Record row;
            stringstream fline(line);
            string word;
            while (getline(fline, word, ' '))
            {
                int num = stoi(word);
                row.push_back(num);
            }
            p.rows.push_back(row);
        }
        p.rowCount = p.rows.size();
        p.columnCount = p.rows[0].size();
        return p;
    }
}

namespace SortTable
{

    void sort(Table *table, int idx, int mode /*acs = 1, desc = 0*/, const string &final_table_name)
    {
        int file_count = 0;
        int total_runs = 0;
        Page p = Page();
        for (int i = 0; i < table->blockCount;)
        {

            vector<Record> data;
            for (int c = 0; c < BUFFER_SIZE - 1 && i < table->blockCount; c++)
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

            int page_index = 0;
            for (auto &record : data)
            {
                InsertRecordIntoPage(p, record, page_index, 0, total_runs);
            }
            if (!p.rows.empty() and !p.rows[0].empty() and p.rows.size())
            {
                p.writePage();
                p.rows.clear();
                page_index++;
                p.rowCount = 0;
                p.columnCount = 0;
            }
            total_runs++;
        }

        int iteration = 1;
        int degree = BUFFER_SIZE - 1;
        struct PageInfo
        {

            Page p;
            int run_number;
            int page_number;
            int cur_row;
        };
        // return;
        // int counter = 0;
        while (total_runs > 1)
        {

            int total_runs_next = 0;
            int cur_run = -1;
            int my_run = -1;
            for (int i = 0; i < total_runs;)
            {

                my_run++;
                vector<PageInfo> pages;
                Page write_page = Page();
                int write_page_index = 0;
                using T = pair<int, PageInfo>;
                auto comp = [&mode](T &A, T &B)
                { return (A.first < B.first) ^ mode; };
                priority_queue<T, vector<T>, decltype(comp)> que(comp);
                for (int c = 0; c < degree && i < total_runs; c++, i++)
                {
                    cur_run++;
                    Page p = getPage(to_string(iteration - 1) + "_run_" + to_string(cur_run), 0);
                    pages.push_back({p, cur_run, 0, 0});
                }
                // return;
                for (auto &it : pages)
                {
                    if (it.p.rowCount > 0)
                    {
                        que.push({it.p.rows[0][idx], it});
                    }
                }
                while (!que.empty())
                {
                    auto [_, page_info] = que.top();
                    que.pop();
                    InsertRecordIntoPage(write_page, page_info.p.rows[page_info.cur_row], write_page_index, iteration, my_run);
                    page_info.cur_row++;
                    if (page_info.cur_row >= page_info.p.rowCount)
                    {

                        // change page
                        int page_number = page_info.page_number + 1;
                        int run = page_info.run_number;
                        Page p1 = getPage(to_string(iteration - 1) + "_run_" + to_string(run), page_number);
                        page_info.cur_row = 0;
                        page_info.p = p1;
                        page_info.page_number = page_number;
                        if (p1.rowCount == 0)
                        {

                            continue;
                        }
                    }
                    que.push({page_info.p.rows[page_info.cur_row][idx], page_info});
                }
                if (!write_page.rows.empty() and !write_page.rows[0].empty())
                {
                    write_page.writePage();
                    write_page.rows.clear();
                    write_page.rowCount = 0;
                    write_page.columnCount = 0;
                    write_page_index++;
                }
                total_runs_next++;
            }
            total_runs = total_runs_next;
            iteration++;
        }

        string file_name = "../data/temp/sorted.csv";
        ofstream final_file(file_name, ios::out | ios::trunc);
        PushIntoFile(final_file, &table->columns);
        for (int i = 0;; i++)
        {

            Page p1 = getPage(to_string(iteration - 1) + "_run_" + to_string(0), i);
            if (p1.rowCount == 0)
            {
                break;
            }
            for (auto &record : p1.rows)
            {
                PushIntoFile(final_file, &record);
            }
        }
        Table *final_table = new Table();
        final_table->sourceFileName = file_name;
        final_table->tableName = final_table_name;
        final_table->load();
        tableCatalogue.insertTable(final_table);

        system("rm ../data/temp/*run*");
        remove(file_name.c_str());
    }

    //     using T = pair<int, pair<int, Record *>>;
    //     auto comp = [&mode](T &A, T &B)
    //     { return (A < B) ^ mode; };
    //     priority_queue<T, vector<T>, decltype(comp)> que(comp);
    //     string filename = "../data/temp/sorted_output.csv";
    //     ofstream final_file(filename, ios::out);
    //     PushIntoFile(final_file, &table->columns);
    //     int cnt = 0;
    //     for (auto fp : opened_files)
    //     {
    //         Record *new_record = GetRecord(fp);
    //         if (!new_record->empty())
    //             que.push({new_record->at(idx), {cnt++, new_record}});
    //     }
    //     while (!que.empty())
    //     {
    //         auto [value, temp] = que.top();
    //         auto [idx_file, record_ptr] = temp;
    //         PushIntoFile(final_file, record_ptr);
    //         record_ptr = GetRecord(opened_files[idx_file]);
    //         que.pop();
    //         if (!record_ptr->empty())
    //             que.push({record_ptr->at(idx), {idx_file, record_ptr}});
    //     }
    //     final_file.close();

    //     Table *final_table = new Table();
    //     final_table->sourceFileName = filename;
    //     final_table->tableName = final_table_name;
    //     final_table->load();
    //     tableCatalogue.insertTable(final_table);
    //     for (int i = 0; i < file_count; i++)
    //     {
    //         remove(("../data/temp/" + to_string(i + 1) + ".txt").c_str());
    //     }
    //     remove(filename.c_str());
    // }
}

namespace SortTable2
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