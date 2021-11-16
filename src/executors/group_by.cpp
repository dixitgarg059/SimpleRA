#include "../global.h"
/**
 * @brief
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseGROUPBY");
    if (tokenizedQuery.size() != 10 || tokenizedQuery[3] != "BY")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = GROUP_BY;
    parsedQuery.groupResultRelationName = tokenizedQuery[0];
    parsedQuery.groupAttribute = tokenizedQuery[4];
    parsedQuery.groupRelationName = tokenizedQuery[6];
    parsedQuery.groupAggregateOperator = tokenizedQuery[8];
    parsedQuery.groupAggregateAttribute = tokenizedQuery[9];
    
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
    void aggragationHelper(unordered_map<int, int> &m, unordered_map<int, int> &cnt, int key, int value, string op)
    {
       
       if(op == "MAX")
       {
           m[key] = max(m[key], value);
       }
       else if(op == "MIN")
       {
           m[key] = min(m[key], value);
       } 
       else if(op == "AVG" || op == "SUM")
       {
           m[key] += value;
       }
       cnt[key]++;
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
        unordered_map <int, int> groupAggregateMap;
        unordered_map <int, int> groupCount;
        int page_index = 0;

        
        for (int i = 0; i < A->blockCount; i++)
        {
            Page* page = bufferManager.getPage(groupRelationName, i);
            int cnt=0;
            for (auto &record : page->rows)
            {
                if (cnt++ == page->rowCount)
                    break;
                int groupAttributeValue = record[groupAttributeIndex];
                int aggregateAttributeValue = record[aggregateAttributeIndex];
                aggragationHelper(groupAggregateMap, groupCount, groupAttributeValue, aggregateAttributeValue, groupAggregateOperator);
            }
        }
        if(groupAggregateOperator == "AVG")
        {
            for(auto i: groupAggregateMap)
            {
                groupAggregateMap[i.first] /= groupCount[i.first];
            }
        }
        for(auto i: groupAggregateMap)
        {
            cout << i.first << " " << i.second << "\n";
            vector<int> record;
            record.push_back(i.first);
            record.push_back(i.second);
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
    result_columns.push_back(parsedQuery.groupAggregateOperator+parsedQuery.groupAggregateAttribute);
    Table *final_table = new Table(parsedQuery.groupResultRelationName, result_columns);
    tableCatalogue.insertTable(final_table);
    GroupBy(parsedQuery.groupRelationName, parsedQuery.groupAttribute, parsedQuery.groupAggregateOperator, parsedQuery.groupAggregateAttribute,
    parsedQuery.groupResultRelationName, final_table);
    cout << "Grouped table";
    logger.log("executeJOIN");
    return;
}