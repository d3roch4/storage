#include "backend.h"
#include <sstream>

using namespace mor;

namespace storage
{

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

string getTypeDB(const type_index &ti){
    if(ti == typeid(string))
        return "text";
    else if(ti == typeid(int) || ti==typeid(unsigned int))
        return "int";
    else if(ti == typeid(long) || ti==typeid(unsigned long))
        return "bigint";
    else if(ti == typeid(short) || ti==typeid(unsigned short))
        return "smallint";
    else if(ti == typeid(bool))
        return "bit";
    else if(ti == typeid(float))
        return "float";
    else if(ti == typeid(double))
        return "float";
    else if(ti == typeid(chrono::time_point<std::chrono::system_clock>))
        return "TIMESTAMP";

    throw_with_nested(runtime_error(string("getTypeDB: type not implemented for: ")+ti.name()));
}

bool isPrimaryKey(const iField *field)
{
    for(const pair<string, string>& opt: field->options){
        if(opt.first == "attrib" && opt.second == "PK")
            return true;
    }
    return false;
}

bool isIndexed(const iField *field)
{
    for(const pair<string, string>& opt: field->options){
        if(opt.first == "attrib" && opt.second == "INDEXED")
            return true;
    }
    return false;
}

}
