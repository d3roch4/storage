#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include "backend.h"
#include <pqxx/pqxx>
#include <vector>
#include <type_traits>

class PostgreSQL : Backend
{
    pqxx::connection connection;
public:

    static shared_ptr<PostgreSQL> getInstance(string connection="");

    template<class type>
    void create(){
        const type obj;
        Table<type>* table = (Table<type>*) &obj;
        const string& sql = Table<type>::sql_create(table->columns);
        pqxx::work txn(connection);
        pqxx::result r = txn.exec(sql);
        txn.commit();
    }

    template<class type>
    void drop(){
        const type obj;
        string sql = "DROP TABLE IF EXISTS "+Table<type>::table_name;
        pqxx::work txn(connection);
        pqxx::result r = txn.exec(sql);
        txn.commit();
    }

    template<class type>
    void remove(type& bean){
        Table<type>* table = (Table<type>*) &bean;
        pqxx::work txn(connection);
        txn.exec("delete from "+table->table_name+" where "+table->id->name+'='+table->id->to_string());
        txn.commit();
    }

    template<class type>
    void insert(type& bean){
        Table<type>* table = (Table<type>*) &bean;
        pqxx::work txn(connection);
        txn.exec(getSqlInsert(table->table_name, table->columns));
        txn.commit();
    }

    template<class type, class TypeId>
    void update(type& bean, TypeId id){
        Table<type>* table = (Table<type>*) &bean;
        pqxx::work txn(connection);
        txn.exec(getSqlUpdate(table->table_name, table->columns)+" where "+table->id->name+'='+to_string(id));
        txn.commit();
    }


    template<class TypeRet>
    TypeRet find_list(string where)
    {
        typedef typename TypeRet:: value_type TypeBean;
        static_assert(std::is_base_of<Table<TypeBean>, TypeBean>::value, "TypeRet is not a list<bean> valid");
        pqxx::work txn(connection);
        TypeRet ret;

        Table<TypeBean>* table;
        pqxx::result r = txn.exec("select * from "+Table<TypeBean>::table_name+" where "+where);
        for(const auto& row: r){
            TypeBean obj;
            table = (Table<TypeBean>*) &obj ;
            for(auto& col: table->columns){
                col->setValue(row.at(r.column_number(col->name)).c_str());
            }
            ret.emplace_back(obj);
        }
        return ret;
    }

    template<class TypeRet, class TypeParam>
    TypeRet find(TypeParam param)
    {
        TypeRet ret;
        Table<TypeRet>* table = (Table<TypeRet>*) &ret;
        return find<TypeRet>(table->id->name+'='+to_string(param));
    }

    template<class TypeRet>
    TypeRet find(string where)
    {
        static_assert(std::is_base_of<Table<TypeRet>, TypeRet>::value, "TypeRet is not a bean valid");
        pqxx::work txn(connection);
        TypeRet ret;
        Table<TypeRet>* table = (Table<TypeRet>*) &ret ;

        pqxx::result r = txn.exec("select * from "+table->table_name+" where "+where);
        for(const auto& row: r){
            for(auto& col: table->columns){
                col->setValue(row.at(r.column_number(col->name)).c_str());
            }
        }
        return ret;
    }

private:
    static shared_ptr<PostgreSQL> instance;
    PostgreSQL(string connection);
};

#endif // POSTGRESQL_H
