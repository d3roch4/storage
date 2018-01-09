#include "postgresql.h"
#include "entity.h"

static void noticeReceiver(void *arg, const PGresult *res)
{
    // faz nada.
}

PostgreSQL::PostgreSQL()
{
}



void PostgreSQL::open(const string &connection)
{
    conn = PQconnectdb(connection.c_str());
    if (PQstatus(conn) == CONNECTION_BAD){
        throw_with_nested(runtime_error(PQerrorMessage(conn)));
    }

    PQsetNoticeReceiver(conn, noticeReceiver, NULL);
}

void PostgreSQL::close()
{
    // close the connection to the database and cleanup
    PQfinish(conn);
}

void PostgreSQL::exec_sql(const string &sql, const vector<unique_ptr<iField> > &columns)
{
    open(get_connection_str());
    PGresult* res = PQexec(conn, sql.c_str());
    verifyResult(res, conn, sql);

    for(int i=0; i<PQntuples(res); i++) {
        for(auto& col: columns){
            col->setValue(PQgetvalue(res, i, PQfnumber(res, col->name.c_str())));
        }
    }

    PQclear(res);
    close();
}

string PostgreSQL::getSqlInsert(const string &entity_name, vector<unique_ptr<iField> > &columns)
{
    return Backend<PostgreSQL>::getSqlInsert(entity_name, columns) + " RETURNING "+getListPK(columns);
}

void verifyResult(PGresult* res, PGconn *conn, const string &sql){
    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
    {
        string erro = PQerrorMessage(conn);
        throw_with_nested(runtime_error("PostgreSQL: "+erro+"\n\tSQL: "+sql) );
    }
}
