#include "orm.h"
#include "postgresql.h"


string getTypeDB(const type_info &ti){
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

string tolower_str(string&& str){
    for(int i=0; i<str.size(); i++)
        str[i] = tolower(str[i]);

    return str;
}
