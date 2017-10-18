#include "orm.h"
#include "postgresql.h"
#include <chrono>



Column::Column(string name, string type_db, const type_info &type_c, void *point_var, PropertyColumn prop) {
    this->name = name;
    this->type_db = type_db;
    this->type_c = type_c.hash_code();
    this->point_var = point_var;
    this->prop = prop;
}

string Column::to_string() const
{
    if(type_c == typeid(string).hash_code())
        return *(string*)point_var;
    else if(type_c == typeid(int).hash_code())
        return std::to_string(*(int*)point_var);
    else if(type_c == typeid(long).hash_code())
        return std::to_string(*(long*)point_var);
    else if(type_c == typeid(unsigned long).hash_code())
        return std::to_string(*(unsigned long*)point_var);
    else if(type_c == typeid(short).hash_code())
        return std::to_string(*(short*)point_var);
    else if(type_c == typeid(bool).hash_code())
        return std::to_string(*(bool*)point_var);
    else if(type_c == typeid(float).hash_code())
        return std::to_string(*(float*)point_var);
    else if(type_c == typeid(double).hash_code())
        return std::to_string(*(double*)point_var);
    else if(type_c == typeid(chrono::time_point<std::chrono::system_clock>).hash_code()){
        std::time_t tp= chrono::system_clock::to_time_t(*(chrono::time_point<std::chrono::system_clock>*)point_var);
        return std::asctime(std::gmtime(&tp));
    }

    throw_with_nested(runtime_error("Column::to_string: type not implemented in Column "+this->name));
}

string getTypeDB(const type_info &ti){
    if(ti == typeid(string))
        return "text";
    else if(ti == typeid(int))
        return "int";
    else if(ti == typeid(long) || ti==typeid(unsigned long))
        return "bigint";
    else if(ti == typeid(short))
        return "smallint";
    else if(ti == typeid(bool))
        return "bit";
    else if(ti == typeid(float))
        return "float";
    else if(ti == typeid(double))
        return "float";
    else if(ti == typeid(chrono::time_point<std::chrono::system_clock>))
        return "TIMESTAMP";

    throw_with_nested(runtime_error("getTypeDB: type not implemented"));
}

string tolower_str(string&& str){
    for(int i=0; i<str.size(); i++)
        str[i] = tolower(str[i]);

    return str;
}
