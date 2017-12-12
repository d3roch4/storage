#ifndef BACKEND_H
#define BACKEND_H

#include <memory>
#include "mor.h"

using namespace std;


enum Comparator {
    EQUAL,
    DIFFERENT,
    BIGGER_THEN,
    LESS_THAN,
    LIKE,
    ARRAY_JSON,
};

struct Condition {
    string col;
    Comparator comparator;
    string val;

    template<class type>
    Condition operator==( type arg){
        this->comparator = Comparator::EQUAL;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator!=( type arg){
        this->comparator = Comparator::DIFFERENT;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator>( type arg){
        this->comparator = Comparator::BIGGER_THEN;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator<( type arg){
        this->comparator = Comparator::LESS_THAN;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator % ( type arg){
        this->comparator = Comparator::LIKE;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator |( type arg){
        this->comparator = Comparator::ARRAY_JSON;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }
};

//template<typename type>

template<class type>
Condition condition(string col, Comparator comparator, type val){
    string str; stringstream ss; ss<<val; ss >> str;
    return Condition{col, comparator, str};
}

enum Operator {
    AND,
    OR
};

class Backend
{
public:
    Backend();

    bool isPrimaryKey(const iColumn* col, const vector<string>& vecPk);

    template<class type>
    string getSqlInsert(Entity<type>* table){
        std::stringstream sql;
        sql << "INSERT INTO " << table->_entity_name << '(';
        for(int i=0; i<table->_columns.size(); i++){
            const iColumn* col = table->_columns.at(i).get();
            if(col->prop==PrimaryKey && col->isNull())
                continue;
            sql << col->name;
            if(i<table->_columns.size()-1)
                sql << ", ";
        }
        sql << ") VALUES (";
        for(int i=0; i<table->_columns.size(); i++){
            const iColumn* col = table->_columns.at(i).get();
            if(col->prop==PrimaryKey && col->isNull())
                continue;
            const string& val = col->getValue();
            sql << '\'' << val << '\'';
            if(i<table->_columns.size()-1)
                sql << ", ";
        }
        sql << ')';
        return sql.str();
    }
    string getSqlUpdate(const string& table, const vector<unique_ptr<iColumn> > &columns);

    template<class type>
    string getWherePK(Entity<type>* table){
        string where;
        for(int i=0; i<table->_columns.size(); i++){
            iColumn* col = table->_columns.at(i).get();
            if(col->prop == PrimaryKey)
                where += col->name +"='"+col->getValue()+"' AND ";
        }

        if(where.size())
            where[where.size()-4] = '\0';
        return where;
    }

    template<class type>
    string getListPK(Entity<type>* table){
        string pks;
        for(int i=0; i<table->_columns.size(); i++){
            iColumn* col = table->_columns.at(i).get();
            if(col->prop == PrimaryKey)
                pks += col->name + ", ";
        }
        if(pks.size())
            pks[pks.size()-2] = '\0';
        return pks;
    }
};


string to_string(const Operator& ope);

string to_string(const Condition &condition);

template <typename T> string where(const T& t) {
    string str = to_string(t);
    return str;
}

template <typename First, typename... Rest> string where(const First& first, const Rest&... rest) {
    string str = to_string(first);
    str += where(rest...); // recursive call using pack expansion syntax
    return str;
}

#endif // BACKEND_H
