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


string to_string(const Operator &ope){
    string str;
    switch (ope) {
    case AND:
        str+= " AND ";
        break;
    case OR:
        str += " OR ";
        break;
    }
    return str;
}


string to_string(const Condition &condition){
    stringstream str;
    switch (condition.comparator) {
    case LIKE:
        str << condition.col<<" like '%"<<condition.val<<"%'";
        break;
    case ARRAY_JSON:
        str<<condition.col << " @> " << condition.val;
        break;
    case EQUAL:
        str << condition.col << "='" << condition.val << "'";
        break;
    case DIFFERENT:
        str << condition.col << "<>'" << condition.val << "'";
        break;
    case BIGGER_THEN:
        str << condition.col << ">'" << condition.val << "'";
        break;
    case LESS_THAN:
        str << condition.col << "<'" << condition.val << "'";
        break;
    }
    return str.str();
}
