#ifndef ORM_H
#define ORM_H

#include <iostream>
#include <vector>
#include <typeinfo>
#include <memory>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace std;

enum PropertyColumn
{
    PrimaryKey,
    ForeignKey,
    Data,
    Index,
};

struct iColumn
{
    PropertyColumn prop;
    string name;
    string type_db;

    void setProperties(string name, string type_db, PropertyColumn prop)
    {
        this->name = name;
        this->type_db = type_db;
        this->prop = prop;
    }

    virtual string to_string() const = 0;
    virtual void setValue(const char* value) = 0;
    virtual bool isNull() const = 0;
};

template<class type>
struct Column : iColumn
{
    type& value;

    Column(type& data) :
        value{data} {
    }

    string to_string() const {
        return std::to_string(value);
    }

    void setValue(const char* str){
        stringstream ss(str);
        ss >> value;
    }

    bool isNull() const{
        return value==0;
    }
};

template<>
struct Column<string> : iColumn
{
    string& value;

    Column(string& str) :
        value(str){
    }

    string to_string() const {
        return value;
    }
    void setValue(const char* str){
        value = str;
    }    
    bool isNull() const{
        return value.empty();
    }
};

template<>
struct Column<chrono::time_point<std::chrono::system_clock>> : iColumn
{
    chrono::time_point<std::chrono::system_clock>& value;

    Column(chrono::time_point<std::chrono::system_clock>& date) :
        value(date) {
    }

    string to_string() const {
        std::time_t tp= chrono::system_clock::to_time_t(value);
        return std::asctime(std::gmtime(&tp));
    }
    void setValue(const char* str){
        struct std::tm tm;
        std::istringstream ss(str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H-%M-%S"); // or just %T in this case
        std::time_t time = mktime(&tm);
        value = chrono::system_clock::from_time_t(time);
    }
    bool isNull() const {
        return value==chrono::time_point<std::chrono::system_clock>{};
    }
};

 string getTypeDB(const type_info& ti);

 string tolower_str(string&& str);

template<class type>
struct Table
{
    iColumn* id;
    vector<shared_ptr<iColumn>> columns; //  coluna: name, type
    static string table_name;

    Table(const string& table = tolower_str(string(typeid(type).name()+1)) ){
        table_name = table;
    }

    template<class tVar>
    void column(tVar& var, string name, PropertyColumn prop=PropertyColumn::Data, string type_db = getTypeDB(typeid(tVar)) ){
        columns.emplace_back(new Column<tVar>(var));
        columns.back()->setProperties(name, type_db, prop);
        if(prop == PropertyColumn::PrimaryKey)
            id = columns.back().get();
    }

    static string sql_create(vector<shared_ptr<iColumn>>& columns){
        vector<iColumn*> vecIndex;
        string sql_create = "CREATE TABLE IF NOT EXISTS "+table_name+"(\n";
        for(int i=0; i<columns.size(); i++){
            iColumn* col = columns[i].get();
            sql_create += col->name +' '+col->type_db;
            sql_create += (col->prop==PrimaryKey)?" NOT NULL PRIMARY KEY":"";
            if(i<columns.size()-1)
                sql_create+=",\n";
            if(col->prop == Index)
                vecIndex.push_back(col);
        }
        sql_create += "\n);\n";
        for(iColumn*& col: vecIndex)
            sql_create += "CREATE INDEX IF NOT EXISTS idx_"+col->name+" ON "+table_name+'('+col->name+");\n";

        return sql_create;
    }
};

template<class type>
string Table<type>::table_name;


#endif // ORM_H
