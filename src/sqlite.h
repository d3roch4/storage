#ifndef SQLITE_H
#define SQLITE_H

#include "backend.h"
#include <sqlite3.h>

class SQLite : public Backend<SQLite>
{
    sqlite3 *db;
public:
    SQLite();

    void open(const string& connection);
    void close();

    void exec_sql(const string& sql, const vector<unique_ptr<iColumn>>& columns={});

    template<class TypeRet>
    TypeRet exec_sql(const string& sql)
    {
        open(get_connection_str());
        close();
    }
};

#endif // SQLITE_H
