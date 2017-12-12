#include "postgresql.h"
#include "mor.h"

shared_ptr<PostgreSQL> PostgreSQL::instance;

static void noticeReceiver(void *arg, const PGresult *res)
{
    // faz nada.
}

PostgreSQL::PostgreSQL(string connectionStr)
{
    conn = PQconnectdb(connectionStr.c_str());
    if (PQstatus(conn) == CONNECTION_BAD){
        throw_with_nested(runtime_error(PQerrorMessage(conn)));
    }

    PQsetNoticeReceiver(conn, noticeReceiver, NULL);
}

shared_ptr<PostgreSQL> PostgreSQL::getInstance(string connection)
{
    if(instance == nullptr)
        instance = shared_ptr<PostgreSQL>(new PostgreSQL(connection));
    return instance;
}

PostgreSQL::~PostgreSQL()
{
    // close the connection to the database and cleanup
    PQfinish(conn);
}


void verifyResult(PGresult* res, PGconn *conn){
    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
    {
        string erro = PQerrorMessage(conn);
        throw_with_nested(runtime_error("PostgreSQL: "+erro) );
    }
}
