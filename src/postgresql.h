#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include "backend.h"
#include <postgresql/libpq-fe.h>
#include <vector>
#include <type_traits>

void verifyResult(PGresult* res, PGconn *conn);

class PostgreSQL : Backend
{
    PGconn     *conn;
public:

    static shared_ptr<PostgreSQL> getInstance(string connection="");

    template<class type>
    void create(){
        const type obj;
        Table<type>* table = (Table<type>*) &obj;
        const string& sql = Table<type>::sql_create(table->columns);
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);
        PQclear(res);

    }

    template<class type>
    void drop(){
        const type obj;
        string sql = "DROP TABLE IF EXISTS "+Table<type>::table_name;
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);
        PQclear(res);
    }

    template<class type>
    void remove(type& bean){
        Table<type>* table = (Table<type>*) &bean;
        string sql = "delete from "+table->table_name+" where "+table->id->name+'='+table->id->to_string();
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);
        PQclear(res);
    }

    template<class type>
    void insert(type& bean){
        Table<type>* table = (Table<type>*) &bean;
        PGresult* res = PQexec(conn, getSqlInsert(table->table_name, table->columns).c_str());
        verifyResult(res, conn);
        PQclear(res);
    }

    template<class type, class TypeId>
    void update(type& bean, TypeId id){
        Table<type>* table = (Table<type>*) &bean;
        PGresult* res = PQexec(conn, (getSqlUpdate(table->table_name, table->columns)+" where "+table->id->name+'='+to_string(id)).c_str());
        verifyResult(res, conn);
        PQclear(res);
    }


    template<class TypeRet>
    TypeRet find_list(string where)
    {
        typedef typename TypeRet:: value_type TypeBean;
        static_assert(std::is_base_of<Table<TypeBean>, TypeBean>::value, "TypeRet is not a list<bean> valid");
        TypeRet ret;

        Table<TypeBean>* table;
        string sql = "select * from "+Table<TypeBean>::table_name+" where "+where;
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);

        int rows = PQntuples(res);
        for(int i=0; i<rows; i++) {
            TypeBean obj;
            table = (Table<TypeBean>*) &obj ;
            for(auto& col: table->columns){
                col->setValue(PQgetvalue(res, i, PQfnumber(res, col->name.c_str())));
            }
            ret.emplace_back(obj);
        }
        PQclear(res);
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
        TypeRet ret;
        Table<TypeRet>* table = (Table<TypeRet>*) &ret ;

        string sql = "select * from "+table->table_name+" where "+where;
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);

        int rows = PQntuples(res);
        for(int i=0; i<rows; i++) {
            for(auto& col: table->columns){
                col->setValue(PQgetvalue(res, i, PQfnumber(res, col->name.c_str())));
            }
        }
        PQclear(res);
        return ret;
    }

    ~PostgreSQL();
private:
    static shared_ptr<PostgreSQL> instance;
    PostgreSQL(string connection);
};

#endif // POSTGRESQL_H
