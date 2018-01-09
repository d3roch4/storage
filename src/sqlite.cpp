#include "sqlite.h"

SQLite::SQLite()
{
}


void SQLite::open(const string &connection)
{
    int rc = sqlite3_open(connection.c_str(), &db);
    if( rc )
        throw runtime_error(string("SQLite::open: ")+ sqlite3_errmsg(db));
}

void SQLite::close()
{
    int rc = sqlite3_close(db);
    if( rc )
        throw runtime_error(string("SQLite::close: ")+ sqlite3_errmsg(db));
}

static int callback(void *data, int argc, char **argv, char **azColName) {
    vector<unique_ptr<iField> >* columns = (vector<unique_ptr<iField> >*)data;
    int i;
    for(i = 0; i<argc; i++) {
      for(auto& col: *columns){
          if(col->name == azColName[i]){
              col->setValue(argv[i]);
              break;
          }
      }
      //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    return 0;
}

void SQLite::exec_sql(const string &sql, const vector<unique_ptr<iField> > &columns)
{
    open(get_connection_str());
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql.c_str(), callback, (void*)&columns, &zErrMsg);
    if( rc != SQLITE_OK ){
        string erro = "SQLite::exec_sql: ";
        erro += zErrMsg;
        erro += "\n\tsql: "+sql;
        sqlite3_free(zErrMsg);
        throw runtime_error(erro);
    }
    close();
}
