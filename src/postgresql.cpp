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

void PostgreSQL::setValueColumn(Column& col, const pqxx::tuple& r)
{
    if(col.type_c == typeid(int).hash_code())
        col.setValue(r.at(r.column_number(col.name)).as<int>());
    else if(col.type_c == typeid(short).hash_code())
        col.setValue(r.at(r.column_number(col.name)).as<short>());
    else if(col.type_c == typeid(long).hash_code())
        col.setValue(r.at(r.column_number(col.name)).as<long>());
    else if(col.type_c == typeid(unsigned long).hash_code())
        col.setValue(r.at(r.column_number(col.name)).as<unsigned long>());
    else if(col.type_c == typeid(string).hash_code())
        col.setValue(r.at(r.column_number(col.name)).as<string>());
    else if(col.type_c == typeid(float).hash_code())
        col.setValue(r.at(r.column_number(col.name)).as<float>());
    else if(col.type_c == typeid(double).hash_code())
        col.setValue(r.at(r.column_number(col.name)).as<double>());
    else
        throw_with_nested(runtime_error("PostgreSQL::setValueColumn: type unknow for Column "+col.name));
}

