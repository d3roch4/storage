#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include "backend.h"
#include <postgresql/libpq-fe.h>
#include <vector>
#include <type_traits>
#include "mor/entity.h"
#include <mutex>
#include <d3util/stacktrace.h>
#include <d3util/logger.h>

namespace storage
{

class PostgreSQL : public Backend<PostgreSQL>
{
    struct ConnectionManager{
        struct Connection
        {
            bool in_use=false;
            PGconn* pgconn=0;
            ~Connection();
        };
        string connection_string;
        vector<Connection> vecConn;
        std::mutex mtx;

        PGconn* try_get();
        PGconn* get();
        void release(PGconn* conn);
    };
    mutable ConnectionManager connection_;
public:

    PostgreSQL();

    void connection(string connection_string, short count=4);

    string exec_sql(const string& sql, const vector<shared_ptr<mor::iField>>& columns={}, const vector<mor::DescField>& descs={}) const;

    template<class TypeRet>
    TypeRet exec_sql(const string& sql)
    {
        PGconn* conn = connection_.get();
        typedef typename TypeRet:: value_type TypeBean;
        TypeRet ret;
        PGresult* res = PQexec(conn, sql.c_str());
        bool ok = verifyResult(res);
        if(ok){
            mor::Entity<TypeBean>* table;
            int rows = PQntuples(res);
            for(int l=0; l<rows; l++) {
                TypeBean obj;
                table = (mor::Entity<TypeBean>*) &obj ;
                for(int c=0; c<table->_fields.size(); c++){
                    table->_fields[c]->setValue(PQgetvalue(res, l, PQfnumber(res, table->_desc_fields[c].name.c_str())), table->_desc_fields[c]);
                }
                ret.emplace_back(std::move(obj));
            }
            PQclear(res);
        }
        connection_.release(conn);
        if(!ok)
            throw_with_trace( runtime_error("PostgreSQL::exec_sql "+string(PQerrorMessage(conn))+"\n\tSQL: "+sql) );
        return ret;
    }

    template<class TypeBean>
    void exec_sql(const string& sql, std::function<void(TypeBean&)> func)
    {
        PGconn* conn = connection_.get();
        PGresult* res = PQexec(conn, sql.c_str());
        bool ok = verifyResult(res);
        if(ok){
            mor::Entity<TypeBean>* table;
            int rows = PQntuples(res);
            for(int l=0; l<rows; l++) {
                TypeBean obj;
                table = (mor::Entity<TypeBean>*) &obj ;
                for(int c=0; c<table->_fields.size(); c++){
                    table->_fields[c]->setValue(PQgetvalue(res, l, PQfnumber(res, table->_desc_fields[c].name.c_str())), table->_desc_fields[c]);
                }
                try{ func(obj);
                }catch(...){
                    ok=false; break;
                }
            }
            PQclear(res);
        }
        connection_.release(conn);
        if(!ok)
            throw_with_trace( runtime_error("PostgreSQL::exec_sql "+string(PQerrorMessage(conn))+"\n\tSQL: "+sql) );
    }
private:
    bool verifyResult(PGresult* res) const;
    string getSqlInsert(const string& entity_name, vector<shared_ptr<mor::iField> > &columns, vector<mor::DescField>& descs) const;
};


bool verifyResult(PGresult* res, PGconn *conn, const string &sql);
}
#endif // POSTGRESQL_H
