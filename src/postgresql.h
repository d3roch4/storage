#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include "backend.h"
#include <postgresql/libpq-fe.h>
#include <vector>
#include <type_traits>

void verifyResult(PGresult* res, PGconn *conn, const string &sql);

class PostgreSQL : public Backend<PostgreSQL>
{
    PGconn     *conn;
public:

    PostgreSQL();

    void open(const string& connection);
    void close();

    void exec_sql(const string& sql, const vector<unique_ptr<iField>>& columns={});

    template<class TypeRet>
    TypeRet exec_sql(const string& sql)
    {
        open(get_connection_str());
        typedef typename TypeRet:: value_type TypeBean;
        TypeRet ret;
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn, sql);

        Entity<TypeBean>* table;
        int rows = PQntuples(res);
        for(int i=0; i<rows; i++) {
            TypeBean obj;
            table = (Entity<TypeBean>*) &obj ;
            for(auto& col: table->_fields){
                col->setValue(PQgetvalue(res, i, PQfnumber(res, col->name.c_str())));
            }
            ret.emplace_back(obj);
        }
        PQclear(res);
        return ret;
        close();
    }

    string getSqlInsert(const string& entity_name, vector<unique_ptr<iField> >& columns);
};

#endif // POSTGRESQL_H
