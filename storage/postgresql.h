#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include "backend.h"
#include <postgresql/libpq-fe.h>
#include <vector>
#include <type_traits>
#include "mor/entity.h"

namespace storage
{

void verifyResult(PGresult* res, PGconn *conn, const string &sql);

class PostgreSQL : public Backend<PostgreSQL>
{
    mutable PGconn     *conn;
public:
    string connection;

    PostgreSQL();

    void open(const string& connection) const ;
    void close() const;

    string exec_sql(const string& sql, const vector<unique_ptr<mor::iField>>& columns={}) const;

    template<class TypeRet>
    TypeRet exec_sql(const string& sql)
    {
        open(connection);
        typedef typename TypeRet:: value_type TypeBean;
        TypeRet ret;
        PGresult* res = PQexec(conn, sql.c_str());
        verifyResult(res, conn, sql);

        mor::Entity<TypeBean>* table;
        int rows = PQntuples(res);
        for(int i=0; i<rows; i++) {
            TypeBean obj;
            table = (mor::Entity<TypeBean>*) &obj ;
            for(auto& col: table->_fields){
                col->setValue(PQgetvalue(res, i, PQfnumber(res, col->name.c_str())));
            }
            ret.emplace_back(obj);
        }
        PQclear(res);
        return ret;
        close();
    }

    string getSqlInsert(const string& entity_name, vector<unique_ptr<mor::iField> >& columns) const;
};

}
#endif // POSTGRESQL_H
