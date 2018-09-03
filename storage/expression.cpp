#include "expression.h"

Expression &Expression::and_(){
    *this += " AND ";
    return *this;
}

Expression &Expression::or_(){
    *this += " OR ";
    return *this;
}

Expression &Expression::like(const std::string &column, const std::string &value){
    *this += column + " LIKE '" + value + "' ";
    return *this;
}

Expression &Expression::in(const std::string &column, const std::string &value){
    *this += column + " IN ("+value+") ";
    return *this;
}

Expression &Expression::not_in(const std::string &column, const std::string &value){
    *this += column + " NOT IN ("+value+") ";
    return *this;
}

Expression &Expression::not_null(const std::string &column){
    *this += column + " IS NOT NULL ";
    return *this;
}

Expression &Expression::is_null(const std::string &column){
    *this += column + " IS NULL ";
    return *this;
}


