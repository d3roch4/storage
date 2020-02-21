#include "functions.h"
#include <string>

namespace storage {

std::string typeName(const std::type_info &ti, std::string& name, Entity* entity){    
    if(ti == typeid(std::string))
        return "text";
    else if(ti == typeid(char) || ti==typeid(unsigned char))
        return "char";
    else if(ti == typeid(short) || ti==typeid(unsigned short))
        return "smallint";
    else if(ti == typeid(int) || ti==typeid(unsigned int))
        return "int";
    else if(ti == typeid(long) || ti==typeid(unsigned long))
        return "bigint";
    else if(ti == typeid(long long) || ti==typeid(unsigned long long))
        return "bigint";
    else if(ti == typeid(bool))
        return "bit(1)";
    else if(ti == typeid(float))
        return "float";
    else if(ti == typeid(double))
        return "float";
    else if(ti == typeid(std::chrono::time_point<std::chrono::system_clock>))
        return "TIMESTAMP";
    else{
        return "";
    }

    throw_with_trace(std::runtime_error("getTypeDB: type not implemented for: "+entity->name+"::"+name));
}

}
