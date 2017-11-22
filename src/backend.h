#ifndef BACKEND_H
#define BACKEND_H

#include <memory>
#include "orm.h"

using namespace std;

class Backend
{
public:
    Backend();

    bool isPrimaryKey(const iColumn* col, const vector<string>& vecPk);

    template<class type>
    string getSqlInsert(Table<type>* table){
        std::stringstream sql;
        sql << "INSERT INTO " << table->table_name << '(';
        for(int i=0; i<table->columns.size(); i++){
            const iColumn* col = table->columns.at(i).get();
            if(col->prop==PrimaryKey && col->isNull())
                continue;
            sql << col->name;
            if(i<table->columns.size()-1)
                sql << ", ";
        }
        sql << ") VALUES (";
        for(int i=0; i<table->columns.size(); i++){
            const iColumn* col = table->columns.at(i).get();
            if(col->prop==PrimaryKey && col->isNull())
                continue;
            const string& val = col->getValue();
            sql << '\'' << val << '\'';
            if(i<table->columns.size()-1)
                sql << ", ";
        }
        sql << ')';
        return sql.str();
    }
    string getSqlUpdate(const string& table, const vector<unique_ptr<iColumn> > &columns);

    template<class type>
    string getWherePK(Table<type>* table){
        string where;
        for(int i=0; i<table->columns.size(); i++){
            iColumn* col = table->columns.at(i).get();
            if(col->prop == PrimaryKey)
                where += col->name +"='"+col->getValue()+"' AND ";
        }

        if(where.size())
            where[where.size()-4] = '\0';
        return where;
    }

    template<class type>
    string getListPK(Table<type>* table){
        string pks;
        for(int i=0; i<table->columns.size(); i++){
            iColumn* col = table->columns.at(i).get();
            if(col->prop == PrimaryKey)
                pks += col->name + ", ";
        }
        if(pks.size())
            pks[pks.size()-2] = '\0';
        return pks;
    }
};

#endif // BACKEND_H
