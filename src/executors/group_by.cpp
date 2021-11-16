#include "../global.h"
/**
 * @brief
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseJOIN");
    // if (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "ON")
    // {
    //     cout << "SYNTAC ERROR" << endl;
    //     return false;
    // }
    parsedQuery.queryType = JOIN;
    parsedQuery.groupAttribute = tokenizedQuery[0];
    parsedQuery.groupRelationName = tokenizedQuery[3];
    parsedQuery.groupResultRelationName = tokenizedQuery[4];
    parsedQuery.groupAggregateOperator = tokenizedQuery[6];
    parsedQuery.groupAggregateAttribute = tokenizedQuery[8];

    // string binaryOperator = tokenizedQuery[7];
    // if (binaryOperator == "<")
    //     parsedQuery.joinBinaryOperator = LESS_THAN;
    // else if (binaryOperator == ">")
    //     parsedQuery.joinBinaryOperator = GREATER_THAN;
    // else if (binaryOperator == ">=" || binaryOperator == "=>")
    //     parsedQuery.joinBinaryOperator = GEQ;
    // else if (binaryOperator == "<=" || binaryOperator == "=<")
    //     parsedQuery.joinBinaryOperator = LEQ;
    // else if (binaryOperator == "==")
    //     parsedQuery.joinBinaryOperator = EQUAL;
    // else if (binaryOperator == "!=")
    //     parsedQuery.joinBinaryOperator = NOT_EQUAL;
    // else
    // {
    //     cout << "SYNTAX ERROR" << endl;
    //     return false;
    // }
    // return true;
}

bool semanticParseGROUPBY()
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
                i.second/= groupCount[i.first];
            }
        }
        for(auto i: groupAggregateMap)
        {
            vector<int> record;
            record.push_back(i.first);
            record.push_back(i.second);
            InsertRecordIntoPage(result, record, page_index, final_table);
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
void executeGROUPBY()
{

    vector<string> result_columns;
    result_columns.push_back(parsedQuery.groupAttribute);
    result_columns.push_back(parsedQuery.groupAggregateOperator+parsedQuery.groupAggregateAttribute);
    Table *final_table = new Table(parsedQuery.groupResultRelationName, result_columns);
    tableCatalogue.insertTable(final_table);
    GroupBy(parsedQuery.groupRelationName, parsedQuery.groupAttribute, parsedQuery.groupAggregateOperator, parsedQuery.groupResultRelationName,
     parsedQuery.groupAggregateAttribute, final_table);
    cout << "Grouped table";
    logger.log("executeJOIN");
    return;
}