#include <thread>

#include "postgresql.h"

namespace storage
{

static void noticeReceiver(void *arg, const PGresult *res)
{
    // faz nada.
}

PostgreSQL::PostgreSQL()
{
}

void PostgreSQL::connection(string connection_string, short count)
{
    connection_.connection_string = connection_string;
    connection_.vecConn.resize(count);
}

void PostgreSQL::close()
{
    connection_.vecConn.clear();
}

string PostgreSQL::exec_sql(const string &sql) const
{
#if DEBUG
    clog << __PRETTY_FUNCTION__ << sql << endl;
#endif
    PGconn* conn = connection_.get();
    PGresult* res = PQexec(conn, sql.c_str());
    bool ok = verifyResult(res);
    connection_.release(conn);

    string rowsAffected = PQcmdTuples(res);
    PQclear(res);

    if(!ok){
        clog << "ERRO SQL: " << sql << endl;
        throw_with_trace( runtime_error("PostgreSQL::exec_sql "+string(PQerrorMessage(conn))) );
    }

    return rowsAffected;
}

void PostgreSQL::exec_sql(const string &sql, std::function<void (PGresult *, int, bool&)> callback)
{
#if DEBUG
    LOG_DEBUG << sql;
#endif
    PGconn* conn = connection_.get();
    PGresult* res = PQexec(conn, sql.c_str());
    bool ok = verifyResult(res);
    int rows = PQntuples(res);
    connection_.release(conn);
    if(ok){
        try{
            callback(res, rows, ok);
        }catch(const std::exception& ex){
            PQclear(res);
            throw_with_trace(ex);
        }
    }
    PQclear(res);
    if(!ok){
        string err = "PostgreSQL::exec_sql "+string(PQerrorMessage(conn))+"\n\tSQL: "+sql;
#if DEBUG
        LOG_ERROR << err << endl;
#endif
        throw_with_trace( runtime_error(err) );
    }
}



bool PostgreSQL::verifyResult(PGresult* res) const{
    ExecStatusType status = PQresultStatus(res);
    if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK)
        return true;

    return false;
}


PGconn* open(const string &connection)
{
    PGconn* conn = PQconnectdb(connection.c_str());
    if (PQstatus(conn) == CONNECTION_BAD){
        throw_with_trace( runtime_error(PQerrorMessage(conn)) );
    }

    PQsetNoticeReceiver(conn, noticeReceiver, NULL);
    return conn;
}

PGconn *PostgreSQL::ConnectionManager::try_get()
{
    unique_lock<mutex> lock{mtx};
    PGconn* result=0;
    if(vecConn.empty())
        throw_with_trace(runtime_error("Não existem coneções"));

    for(Connection& conn: vecConn){
        if(conn.in_use == false){
            if(PQstatus(conn.pgconn) != CONNECTION_OK){
                PQfinish(conn.pgconn);
                conn.pgconn = open(connection_string);
            }
            result = conn.pgconn;
            conn.in_use = true;
            break;
        }
    }
    return result;
}

PGconn *PostgreSQL::ConnectionManager::get()
{
    PGconn* result=0;
    while((result=try_get()) == NULL)    {
        std::this_thread::sleep_for(std::chrono::microseconds{300});
    }
    return result;
}

void PostgreSQL::ConnectionManager::release(PGconn *pgconn)
{
    for(Connection& conn: vecConn){
        if(conn.pgconn == pgconn){
            unique_lock<mutex> lock{mtx};
            conn.in_use = false;
            break;
        }
    }
}

storage::PostgreSQL::ConnectionManager::Connection::Connection()
{

}

PostgreSQL::ConnectionManager::Connection::~Connection()
{
    if(pgconn)
        PQfinish(pgconn);
}

}
