#ifndef EXPRESSION_H
#define EXPRESSION_H
#include <string>
#include <sstream>

struct Expression : public std::string{

    Expression& and_();

    Expression& or_();

    Expression& like(const std::string& column, const std::string& value);

    Expression& eq(const std::string& column, const auto& value){
        std::stringstream ss; ss << value;
        *this += column + "='" +ss.str() + "' ";
        return *this;
    }

    Expression& in(const std::string& column, const std::string& value);

    Expression& not_in(const std::string& column, const std::string& value);

    Expression& not_null(const std::string& column);

    Expression& is_null(const std::string& column);
};

#endif // EXPRESSION_H
