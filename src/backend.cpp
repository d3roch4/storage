#include "backend.h"
#include <sstream>

Backend::Backend()
{

}

string Backend::getSqlInsert(const string &table, const vector<Column> &columns)
{
    std::stringstream sql;
    sql << "INSERT INTO " << table << '(';
    for(int i=0; i<columns.size(); i++){
        const Column& col = columns[i];
        sql << col.name;
        if(i<columns.size()-1)
            sql << ", ";
    }
    sql << ") VALUES (";
    for(int i=0; i<columns.size(); i++){
        const Column* col = &columns[i];
        const string& val = col->to_string();
        sql << '\'' << val << '\'';
        if(i<columns.size()-1)
            sql << ", ";
    }
    sql << ')';
    return sql.str();
}

string Backend::getSqlUpdate(const string &table, const vector<Column> &columns)
{
    std::stringstream sql;
    sql << "UPDATE " << table << " SET ";
    for(int i=0; i<columns.size(); i++){
        const Column& col = columns[i];
        sql << col.name << "=\'" << col.to_string() << '\'';;
        if(i<columns.size()-1)
            sql << ", ";
    }
    return sql.str();
}
