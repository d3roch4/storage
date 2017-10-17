#ifndef ORM_H
#define ORM_H

#include <iostream>
#include <vector>
#include <typeinfo>
#include <memory>

using namespace std;

enum PropertyColumn
{
    PrimaryKey,
    ForeignKey,
    Data,
    Index,
};


struct Column
{
    PropertyColumn prop;
    string name;
    string type_db;
    size_t type_c;
    void* point_var;

    Column(){}

    Column(string name, string type_db, const type_info& type_c, void* point_var, PropertyColumn prop);

    string to_string() const;

    template<class type> void setValue(type value){
        if(typeid(type).hash_code() != type_c)
            throw_with_nested(runtime_error("Column::setValue: types not eguals"));
        type* pvar = reinterpret_cast<type*>(point_var);
        *pvar = value;
    }
};

 string getTypeDB(const type_info& ti);

template<class type>
struct Table
{
    Column id;
    vector<Column> columns; //  coluna: name, type
    static string table_name;
    static string sql_create;
    static bool created;

    Table(const string& table = typeid(type).name()+1){
        table_name = table;
    }

    template<class tVar>
    void column(tVar& var, string name, PropertyColumn prop=PropertyColumn::Data, string type_db = getTypeDB(typeid(tVar)) ){
        Column column{name, type_db, typeid(tVar),  &var, prop};
        columns.push_back(column);
        if(prop == PropertyColumn::PrimaryKey)
            id = column;
        rebuild_sqls();
    }

private:
    void rebuild_sqls(){
        vector<Column> vecIndex;
        sql_create = "CREATE TABLE IF NOT EXISTS "+table_name+"(\n";
        for(int i=0; i<columns.size(); i++){
            Column& col = columns[i];
            sql_create += col.name +' '+col.type_db;
            sql_create += (col.prop==PrimaryKey)?" NOT NULL PRIMARY KEY":"";
            if(i<columns.size()-1)
                sql_create+=",\n";
            if(col.prop == Index)
                vecIndex.push_back(col);
        }
        sql_create += "\n);\n";
        for(Column& col: vecIndex)
            sql_create += "CREATE INDEX idx_"+col.name+" ON "+table_name+'('+col.name+");\n";
    }
};


template<class type>
string Table<type>::table_name;

template<class type>
string Table<type>::sql_create;

template<class type>
bool Table<type>::created = false;

template<class type_backend>
class Persistence
{
    type_backend backend;
public:
    Persistence(string connection) :
        backend{connection} {
    }

    template<class TypeRet, class TypeId>
    shared_ptr<TypeRet> find(TypeId id){
        return backend.find<TypeRet>(id);
    }
};

#endif // ORM_H
