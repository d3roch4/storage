#include <thread>

#include "postgresql.h"

using namespace mor;

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

void PostgreSQL::setValues(PGresult* res, const int& row, int& coll, mor::iEntity* entity, int nColl) const
{
    vector<mor::iEntity*> entityesJoin;
    auto&& vecDesc = entity->_get_desc_fields();
    for(int j=0; j<vecDesc.size() && coll<nColl; j++){
        DescField& desc = entity->_get_desc_fields()[j];
        auto&& ref = desc.options.find("reference");
        if(ref != desc.options.end())
            entityesJoin.push_back((mor::iEntity*)entity->_get_fields()[j]->value);
        else
            entity->_get_fields()[j]->setValue(PQgetvalue(res, row, coll), desc);
        coll++;
    }
    for(iEntity* ent: entityesJoin)
        setValues(res, row, coll, ent, nColl);
}

string PostgreSQL::exec_sql(const string& sql, mor::iEntity* entity) const
{
    PGconn* conn = connection_.get();
    PGresult* res = PQexec(conn, sql.c_str());
    bool ok = verifyResult(res);
    connection_.release(conn);

    if(ok && PQntuples(res)>0 && entity!=NULL){
        int coll=0;
        setValues(res, 0, coll, entity, PQnfields(res));
    }

    string rowsAffected = PQcmdTuples(res);
    PQclear(res);

    if(!ok)
        throw_with_trace( runtime_error("PostgreSQL::exec_sql "+string(PQerrorMessage(conn))+"\n\tSQL: "+sql) );

    return rowsAffected;
}

void PostgreSQL::exec_sql(const string &sql, std::function<void (PGresult *, int, bool&)> callback)
{
    clog << __PRETTY_FUNCTION__ << sql << endl;
    PGconn* conn = connection_.get();
    PGresult* res = PQexec(conn, sql.c_str());
    bool ok = verifyResult(res);
    connection_.release(conn);
    if(ok){
        int rows = PQntuples(res);
        try{
            callback(res, rows, ok);
        }catch(const std::exception& ex){
            PQclear(res);
            throw_with_trace(ex);
        }
    }
    PQclear(res);
    if(!ok)
        throw_with_trace( runtime_error("PostgreSQL::exec_sql "+string(PQerrorMessage(conn))+"\n\tSQL: "+sql) );
}

string PostgreSQL::getSqlInsert(const string &entity_name, vector<shared_ptr<iField> > &columns, vector<DescField>& descs) const
{
    const string& pks = getListPK(descs);
    return Backend<PostgreSQL>::getSqlInsert(entity_name, columns, descs) + (pks.size()?" RETURNING "+pks:"");
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
