#include "backend.h"
#include <sstream>

Backend::Backend()
{

}


string Backend::getSqlUpdate(const string &table, const vector<unique_ptr<iColumn>> &columns)
{
    std::stringstream sql;
    sql << "UPDATE " << table << " SET ";
    for(int i=0; i<columns.size(); i++){
        const iColumn* col = columns[i].get();
        sql << col->name << "=\'" << col->getValue() << '\'';
        if(i<columns.size()-1)
            sql << ", ";
    }
    return sql.str();
}

