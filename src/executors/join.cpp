#include "../global.h"
/**
 * @brief
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "ON")
    {
        cout << "SYNTAC ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[3];
    parsedQuery.joinSecondRelationName = tokenizedQuery[4];
    parsedQuery.joinFirstColumnName = tokenizedQuery[6];
    parsedQuery.joinSecondColumnName = tokenizedQuery[8];

    string binaryOperator = tokenizedQuery[7];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
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
    void InsertRecordIntoPage(Page &p, const vector<int> &record1, const vector<int> &record2, int &page_index, Table *final_table)
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
        }
    }
    void Join(string name1, string name2, string column1, string column2, string result_relation, Table *final_table)
    {

        Page result = Page();
        result.tableName = result_relation;
        vector<Page *> outer;
        int idx = 0;

        Table *B = tableCatalogue.getTable(name2);
        Table *A = tableCatalogue.getTable(name1);
        int idx1 = A->getColumnIndex(column1);
        int idx2 = B->getColumnIndex(column2);

        int page_index = 0;
        while (1)
        {

            for (int i = 0; i < min(BLOCK_COUNT - 2, A->blockCount - idx); i++)
            {
                outer.push_back(bufferManager.getPage(name1, idx));
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

                    for (const auto &page : outer)
                    {

                        int cnt = 0;
                        for (auto &record1 : page->rows)
                        {

                            if (cnt++ == page->rowCount)
                                break;
                            if (record1.size() <= idx1)
                                continue;
                            int val1 = record1[idx1];
                            if (val1 == val2)
                            {
                                InsertRecordIntoPage(result, record1, record2, page_index, final_table);
                            }
                        }
                    }
                }
                bufferManager.PopPool();
            }
            bufferManager.clear();
            outer.clear();
        }
        if (!result.rows.empty())
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

}
void executeJOIN()
{

    Table *A = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *B = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);
    vector<string> result_columns = A->columns;
    for (auto it : B->columns)
        result_columns.push_back(it);
    Table *final_table = new Table(parsedQuery.joinResultRelationName, result_columns);
    tableCatalogue.insertTable(final_table);
    Join(parsedQuery.joinFirstRelationName, parsedQuery.joinSecondRelationName, parsedQuery.joinFirstColumnName, parsedQuery.joinSecondColumnName, parsedQuery.joinResultRelationName, final_table);
    cout << "joined table";
    logger.log("executeJOIN");
    return;
}