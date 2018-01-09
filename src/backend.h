#ifndef BACKEND_H
#define BACKEND_H

#include <memory>
#include "entity.h"

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


bool isPrimaryKey(const iField* field);

bool isIndexed(const iField *field);

template<typename TypeBackend>
class Backend
{
    static string connection;
    static shared_ptr<TypeBackend> instance;
public:


    Backend(){}

    virtual void exec_sql(const string& sql, const vector<unique_ptr<iField>>& columns={}) = 0;
    virtual void open(const string& connection)=0;
    virtual void close()=0;

    string get_connection_str(){
        return connection;
    }

    template<class TypeRet>
    TypeRet exec_sql(const string& sql)
    {
        return ((TypeBackend*)this)->template exec_sql<TypeRet>(sql);
    }

    template<class type>
    void create(){
        const type obj;
        Entity<type>* table = (Entity<type>*) &obj;
        const string& sql = sql_create(table);
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
        const string& sql = (getSqlUpdate(table->_entity_name, table->_fields)+" where "+where);

        exec_sql(sql);
    }

    template<class type>
    void insert(type& bean){
        Entity<type>* table = (Entity<type>*) &bean;
        const string& sql = getSqlInsert(table->_entity_name, table->_fields);

        exec_sql(sql, table->_fields);
    }


    template<class TypeRet>
    TypeRet find(string where)
    {
        static_assert(std::is_base_of<Entity<TypeRet>, TypeRet>::value, "TypeRet is not a bean valid");
        TypeRet ret;
        Entity<TypeRet>* table = (Entity<TypeRet>*) &ret ;

        string sql = "select * from "+table->_entity_name+" where "+where;

        exec_sql(sql, table->_fields);
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

    virtual string getSqlInsert(const string& entity_name, vector<unique_ptr<iField> >& columns){
        std::stringstream sql;
        sql << "INSERT INTO " << entity_name << '(';
        for(int i=0; i<columns.size(); i++){
            const iField* col = columns.at(i).get();
            if(isPrimaryKey(col) && col->isNull())
                continue;
            sql << col->name;
            if(i<columns.size()-1)
                sql << ", ";
        }
        sql << ") VALUES (";
        for(int i=0; i<columns.size(); i++){
            const iField* col = columns.at(i).get();
            if(isPrimaryKey(col) && col->isNull())
                continue;
            const string& val = col->getValue();
            sql << '\'' << val << '\'';
            if(i<columns.size()-1)
                sql << ", ";
        }
        sql << ')';
        return sql.str();
    }

    string getSqlUpdate(const string& table, const vector<unique_ptr<iField> > &columns)
    {
        std::stringstream sql;
        sql << "UPDATE " << table << " SET ";
        for(int i=0; i<columns.size(); i++){
            const iField* col = columns[i].get();
            sql << col->name << "=\'" << col->getValue() << '\'';
            if(i<columns.size()-1)
                sql << ", ";
        }
        return sql.str();
    }

    template<class type>
    string getWherePK(Entity<type>* table){
        string where;
        for(int i=0; i<table->_fields.size(); i++){
            iField* col = table->_fields.at(i).get();
            if(isPrimaryKey(col))
                where += col->name +"='"+col->getValue()+"' AND ";
        }

        if(where.size())
            where[where.size()-4] = '\0';
        return where;
    }

    string getListPK(vector<unique_ptr<iField> >& columns){
        string pks;
        for(int i=0; i<columns.size(); i++){
            iField* col = columns.at(i).get();
            if(isPrimaryKey(col))
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

string getTypeDB(const type_index& ti);

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

template<typename type>
string sql_create(Entity<type>* table){
    short qtdPK=0;
    string sql_create = "CREATE TABLE IF NOT EXISTS "+table->_entity_name+"(\n";
    for(int i=0; i<table->_fields.size(); i++){
        iField* col = table->_fields[i].get();
        if(col->options.find("type_db")==col->options.end())
            col->options["type_db"] = getTypeDB(*col->typeinfo);
        sql_create += col->name +' '+col->options["type_db"];
        sql_create += (col->options["attrib"]=="NotNull")?" NOT NULL ":"";
        if(i<table->_fields.size()-1)
            sql_create+=",\n";
        if(isPrimaryKey(col))
            qtdPK++;
    }
    if(qtdPK){
        sql_create += ", CONSTRAINT PK_"+table->_entity_name+" PRIMARY KEY (";
        for(int i=0; i<table->_fields.size(); i++){
            iField* col = table->_fields[i].get();
            if(isPrimaryKey(col)){
                sql_create += col->name;
                if(i<qtdPK-1)
                    sql_create += ", ";
            }
        }
        sql_create += ")\n";
    }
    if(table->_vecFK.size()){
        for(int i=0; i<table->_vecFK.size(); i++){
            sql_create += ", CONSTRAINT FK_"+table->_vecFK[i].field->name+table->_entity_name+
                " FOREIGN KEY ("+table->_vecFK[i].field->name+") REFERENCES "+table->_vecFK[i].reference;
        }
    }
    sql_create += "\n);\n";
    for(auto& col: table->_fields)
        if(isIndexed(col.get()))
            sql_create += "CREATE INDEX IF NOT EXISTS idx_"+col->name+" ON "+table->_entity_name+'('+col->name+");\n";

    return sql_create;
}

#endif // BACKEND_H
