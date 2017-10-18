#include "postgresql.h"
#include "orm.h"

shared_ptr<PostgreSQL> PostgreSQL::instance;

PostgreSQL::PostgreSQL(string conn) :
    connection{conn}
{
}

shared_ptr<PostgreSQL> PostgreSQL::getInstance(string connection)
{
    if(instance == nullptr)
        instance = shared_ptr<PostgreSQL>(new PostgreSQL(connection));
    return instance;
}


