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

template<typename TypeBackend>
class Backend
{
    static string connection;
    static shared_ptr<TypeBackend> instance;
public:
    Backend(){}

    virtual void exec_sql(const string& sql, const vector<unique_ptr<iColumn>>& columns={}) = 0;
    virtual void open(const string& connection)=0;
    virtual void close()=0;

    string get_connection_str(){
        return connection;
    }

    template<class TypeRet>
    TypeRet exec_sql(const string& sql)
    {
        return ((TypeBackend*)this)->exec_sql<TypeRet>(sql);
    }

    template<class type>
    void create(){
        const type obj;
        Entity<type>* table = (Entity<type>*) &obj;
        const string& sql = table->sql_create(table->_columns);

        exec_sql(sql);
    }

    template<class type>
    void drop(){
        const type obj;
        string sql = "DROP TABLE IF EXISTS "+Entity<type>::_entity_name;

        exec_sql(sql);
    }

    template<class type>
    void remove(type& bean){
        Entity<type>* table = (Entity<type>*) &bean;
        string sql = "delete from "+table->_entity_name+" where "+getWherePK(table);

        exec_sql(sql);
    }

    template<class type>
    void update(type& bean, const string& where){
        Entity<type>* table = (Entity<type>*) &bean;
        const string& sql = (getSqlUpdate(table->_entity_name, table->_columns)+" where "+where);

        exec_sql(sql);
    }

    template<class type>
    void insert(type& bean){
        Entity<type>* table = (Entity<type>*) &bean;
        const string& sql = getSqlInsert(table);// + " RETURNING "+getListPK(table);

        exec_sql(sql, table->_columns);
    }


    template<class TypeRet>
    TypeRet find(string where)
    {
        static_assert(std::is_base_of<Entity<TypeRet>, TypeRet>::value, "TypeRet is not a bean valid");
        TypeRet ret;
        Entity<TypeRet>* table = (Entity<TypeRet>*) &ret ;

        string sql = "select * from "+table->_entity_name+" where "+where;

        exec_sql(sql, table->_columns);
        return ret;
    }

    template<class TypeRet>
    TypeRet find_list(string where)
    {
        typedef typename TypeRet:: value_type TypeBean;
        static_assert(std::is_base_of<Entity<TypeBean>, TypeBean>::value, "TypeRet is not a list<bean> valid");

        string sql = "select * from "+Entity<TypeBean>::_entity_name+" where "+where;

        return exec_sql<TypeRet>(sql);
    }


    static shared_ptr<TypeBackend> getInstance(string connection="")
    {
        if(instance == nullptr){
            instance = shared_ptr<TypeBackend>(new TypeBackend());
            Backend<TypeBackend>::connection = connection;
        }
        return instance;
    }

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

    string getSqlUpdate(const string& table, const vector<unique_ptr<iColumn> > &columns)
    {
        std::stringstream sql;
        sql << "UPDATE " << table << " SET ";
        for(int i=0; i<columns.size(); i++){
            const iColumn* col = columns[i].get();
            sql << col->name << "=\'" << col->getValue() << '\'';
            if(i<columns.size()-1)
                sql << ", ";
        }
        return sql.str();
    }

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

template<typename TypeBackend>
shared_ptr<TypeBackend> Backend<TypeBackend>::instance;

template<typename TypeBackend>
string Backend<TypeBackend>::connection;

string to_string(const Operator& ope);

string to_string(const Condition &condition);

template <typename T>
string where(const T& t) {
    string str = to_string(t);
    return str;
}

template <typename First, typename... Rest>
string where(const First& first, const Rest&... rest) {
    string str = to_string(first);
    str += where(rest...); // recursive call using pack expansion syntax
    return str;
}

#endif // BACKEND_H
