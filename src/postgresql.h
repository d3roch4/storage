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
    void create(type& bean){
        if( ! Table<type>::created ){
            Table<type>::created = true;
            const string& sql = Table<type>::sql_create;
            pqxx::work txn(connection);
            pqxx::result r = txn.exec(sql);
            txn.commit();
        }
    }

    template<class type>
    void remove(type& bean){
        Table<type>* table = (Table<type>*) &bean;
        pqxx::work txn(connection);
        txn.exec("delete from "+table->table_name+" where "+table->id.name+'='+table->id.to_string());
        txn.commit();
    }

    template<class type>
    void insert(type& bean){
        create(bean);
        Table<type>* table = (Table<type>*) &bean;
        pqxx::work txn(connection);
        txn.exec(getSqlInsert(table->table_name, table->columns));
        txn.commit();
    }

    template<class type, class TypeId>
    void update(type& bean, TypeId id){
        create(bean);
        Table<type>* table = (Table<type>*) &bean;
        pqxx::work txn(connection);
        txn.exec(getSqlUpdate(table->table_name, table->columns)+" where "+table->id.name+'='+to_string(id));
        txn.commit();
    }


    template<class TypeRet>
    TypeRet find_list(string where)
    {
        typedef typename TypeRet:: value_type TypeBean;
        static_assert(std::is_base_of<Table<TypeBean>, TypeBean>::value, "TypeRet is not a list<bean> valid");
        pqxx::work txn(connection);
        TypeRet ret;
        TypeBean temp;
        Table<TypeBean>* table = (Table<TypeBean>*) &temp ;

        pqxx::result r = txn.exec("select * from "+table->table_name+" where "+where);
        for(const auto& row: r){
            TypeBean obj;
            table = (Table<TypeBean>*) &obj ;
            for(Column& col: table->columns){
                setValueColumn(col, row);
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
        return find<TypeRet>(table->id.name+'='+to_string(param));
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
            for(Column& col: table->columns){
                setValueColumn(col, row);
            }
        }
        return ret;
    }

private:
    static shared_ptr<PostgreSQL> instance;
    void setValueColumn(Column& col, const pqxx::tuple &r);
    PostgreSQL(string connection);
};

#endif // POSTGRESQL_H
