#include "../global.h"
/**
 * @brief
 * SYNTAX: R <- <new_table> <- GROUP BY <grouping_attribute> FROM <table_name> RETURN
MAX|MIN|SUM|AVG(<attribute>)
 */
bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseGROUPBY");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[3] != "BY" || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "RETURN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = GROUP_BY;
    parsedQuery.groupResultRelationName = tokenizedQuery[0];
    parsedQuery.groupAttribute = tokenizedQuery[4];
    parsedQuery.groupRelationName = tokenizedQuery[6];

    string s = tokenizedQuery[8];
    if (s.size() < 6)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    string s1 = s.substr(0, 3);
    string s2 = s.substr(4, (int)s.size() - 5);
    if (s[3] != '(' || s.back() != ')')
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    if (s1 != "MAX" and s1 != "MIN" and s1 != "AVG" and s1 != "SUM")
    {
        cout << "SYNTAX ERROR: Invalid Aggregate Operation" << endl;
        return false;
    }

    parsedQuery.groupAggregateOperator = s1;
    parsedQuery.groupAggregateAttribute = s2;

    return true;
}

bool semanticParseGROUPBY()
{
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupAttribute, parsedQuery.groupRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.groupAggregateAttribute, parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

namespace
{
    void InsertRecordIntoPage(Page &p, const vector<int> &record, int &page_index, Table *final_table)
    {
        p.rows.push_back(record);
        if (!p.rows.empty() and !p.rows[0].empty() and p.rows.size() >= ((int)BLOCK_SIZE * 1000 / (sizeof(int) * p.rows[0].size())))
        {

            final_table->rowCount += p.rows.size();
            p.rowCount = p.rows.size();
            final_table->rowsPerBlockCount.push_back(p.rows.size());
            p.columnCount = p.rows[0].size();
            p.pageName = "../data/temp/" + p.tableName + "_Page" + to_string(page_index);
            p.writePage();
            p.rows.clear();
            page_index++;
            p.rowCount = 0;
            p.columnCount = 0;
            final_table->blockCount++;
        }
    }
    void aggragationHelper(unordered_map<int, pair<int, int>> &m, int key, int value, string op)
    {

        if (op == "MAX")
        {
            m[key].first = max(m[key].first, value);
        }
        else if (op == "MIN")
        {
            m[key].first = min(m[key].first, value);
        }
        else if (op == "AVG" || op == "SUM")
        {
            m[key].first += value;
        }
        m[key].second++;
        return;
    }
    void GroupBy(string groupRelationName, string groupAttributeName, string groupAggregateOperator, string groupAggregateAttributeName, string result_relation, Table *final_table)
    {

        Page result = Page();
        result.tableName = result_relation;
        vector<Page *> outer;
        int idx = 0;

        Table *A = tableCatalogue.getTable(groupRelationName);
        int groupAttributeIndex = A->getColumnIndex(groupAttributeName);
        int aggregateAttributeIndex = A->getColumnIndex(groupAggregateAttributeName);
        unordered_map<int, pair<int, int>> groupAggregateMap;
        int page_index = 0;

        for (int i = 0; i < A->blockCount; i++)
        {
            Page *page = bufferManager.getPage(groupRelationName, i);
            int cnt = 0;
            for (auto &record : page->rows)
            {
                if (cnt++ == page->rowCount)
                    break;
                int groupAttributeValue = record[groupAttributeIndex];
                int aggregateAttributeValue = record[aggregateAttributeIndex];
                aggragationHelper(groupAggregateMap, groupAttributeValue, aggregateAttributeValue, groupAggregateOperator);
            }
        }
        if (groupAggregateOperator == "AVG")
        {
            for (auto i : groupAggregateMap)
            {
                groupAggregateMap[i.first].first /= groupAggregateMap[i.first].second;
            }
        }
        for (auto i : groupAggregateMap)
        {
            vector<int> record;
            record.push_back(i.first);
            record.push_back(i.second.first);
            InsertRecordIntoPage(result, record, page_index, final_table);
        }
        if (!result.rows.empty())
        {
            cout << result.rows.size();
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
void executeGROUPBY()
{

    vector<string> result_columns;
    result_columns.push_back(parsedQuery.groupAttribute);
    result_columns.push_back(parsedQuery.groupAggregateOperator + parsedQuery.groupAggregateAttribute);
    Table *final_table = new Table(parsedQuery.groupResultRelationName, result_columns);
    // tableCatalogue.insertTable(final_table);
    block_accesses_read = 0;
    block_accesses_write = 0;
    GroupBy(parsedQuery.groupRelationName, parsedQuery.groupAttribute, parsedQuery.groupAggregateOperator, parsedQuery.groupAggregateAttribute,
            parsedQuery.groupResultRelationName, final_table);
    std::cout << "Done group by \n No. of block accesses  for reading = " << block_accesses_read << "\n No. of block accesses for writing = " << block_accesses_write << endl;
    logger.log("executeJOIN");
    if (final_table->rowCount) {
        tableCatalogue.insertTable(final_table);
    } 
    else {
        final_table->unload();
        delete final_table;
        cout << "Empty Table" << endl;
    }
    return;
}