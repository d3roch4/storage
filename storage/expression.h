#ifndef EXPRESSION_H
#define EXPRESSION_H
#include <string>
#include <sstream>

struct Expression : public std::string{

    Expression& and_();

    Expression& or_();

    Expression& like(const std::string& column, const std::string& value);

    template<class V>
    Expression& eq(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        *this += column + "='" +ss.str() + "' ";
        return *this;
    }

    template<class V>
    Expression& nq(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        *this += column + "<>'" +ss.str() + "' ";
        return *this;
    }

    template<class V>
    Expression& gt(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        *this += column + ">'" +ss.str() + "' ";
        return *this;
    }

    template<class V>
    Expression& ge(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        *this += column + ">='" +ss.str() + "' ";
        return *this;
    }

    template<class V>
    Expression& lt(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        *this += column + "<'" +ss.str() + "' ";
        return *this;
    }

    template<class V>
    Expression& le(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        *this += column + "<='" +ss.str() + "' ";
        return *this;
    }

    Expression& in(const std::string& column, const std::string& value);

    Expression& not_in(const std::string& column, const std::string& value);

    Expression& not_null(const std::string& column);

    Expression& is_null(const std::string& column);

    Expression& ps();

    Expression& pe();

};

#endif // EXPRESSION_H
