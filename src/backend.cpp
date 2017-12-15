#include "backend.h"
#include <sstream>



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


