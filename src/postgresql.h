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
        Entity<type>* table = (Entity<type>*) &obj;
        const string& sql = table->sql_create(table->_columns);
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);
        PQclear(res);

    }

    template<class type>
    void drop(){
        const type obj;
        string sql = "DROP TABLE IF EXISTS "+Entity<type>::_entity_name;
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);
        PQclear(res);
    }

    template<class type>
    void remove(type& bean){
        Entity<type>* table = (Entity<type>*) &bean;
        string sql = "delete from "+table->_entity_name+" where "+getWherePK(table);
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);
        PQclear(res);
    }

    template<class type>
    void insert(type& bean){
        Entity<type>* table = (Entity<type>*) &bean;
        const string& sql = getSqlInsert(table) + " RETURNING "+getListPK(table);
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);

        for(int i=0; i<PQntuples(res); i++) {
            for(auto& col: table->_columns){
                if(col->prop == PrimaryKey)
                    col->setValue(PQgetvalue(res, i, PQfnumber(res, col->name.c_str())));
            }
        }

        PQclear(res);
    }

    template<class type>
    void update(type& bean, const string& where){
        Entity<type>* table = (Entity<type>*) &bean;
        const string& sql = (getSqlUpdate(table->_entity_name, table->_columns)+" where "+where);
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);
        PQclear(res);
    }


    template<class TypeRet>
    TypeRet find_list(string where)
    {
        typedef typename TypeRet:: value_type TypeBean;
        static_assert(std::is_base_of<Entity<TypeBean>, TypeBean>::value, "TypeRet is not a list<bean> valid");
        TypeRet ret;

        Entity<TypeBean>* table;
        string sql = "select * from "+Entity<TypeBean>::_entity_name+" where "+where;
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn);

        int rows = PQntuples(res);
        for(int i=0; i<rows; i++) {
            TypeBean obj;
            table = (Entity<TypeBean>*) &obj ;
            for(auto& col: table->_columns){
                col->setValue(PQgetvalue(res, i, PQfnumber(res, col->name.c_str())));
            }
            ret.emplace_back(obj);
        }
        PQclear(res);
        return ret;
    }


    template<class TypeRet>
    TypeRet find(string where)
    {
        static_assert(std::is_base_of<Entity<TypeRet>, TypeRet>::value, "TypeRet is not a bean valid");
        TypeRet ret;
        Entity<TypeRet>* table = (Entity<TypeRet>*) &ret ;

        string sql = "select * from "+table->_entity_name+" where "+where;
        const char* cs = sql.c_str();
        PGresult* res = PQexec(conn, cs);
        verifyResult(res, conn);

        for(int i=0; i<PQntuples(res); i++) {
            for(auto& col: table->_columns){
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
