#ifndef ORM_H
#define ORM_H

#include <iostream>
#include <cstring>
#include <vector>
#include <typeinfo>
#include <memory>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace std;


string getTypeDB(const type_info& ti);

string tolower_str(string&& str);

enum PropertyColumn
{
    NotNull,
    Data,
    PrimaryKey,
    Index,
};

struct iColumn
{
    PropertyColumn prop;
    string name;
    string type_db;
    void* value;

    void setProperties(string name, string type_db, PropertyColumn prop)
    {
        this->name = name;
        this->type_db = type_db;
        this->prop = prop;
    }

    virtual string getValue() const = 0;
    virtual void setValue(const char* value) = 0;
    virtual bool isNull() const = 0;
    virtual unique_ptr<iColumn> copy() = 0;
};

template<class type>
struct Column : iColumn
{

    Column(type& data) {
        value = &data;
    }

    string getValue() const {
        return to_string(*(type*)value);
    }

    void setValue(const char* str){
        stringstream ss(str);
        ss >> *(type*)value;
    }

    bool isNull() const{
        return *(type*)value==0;
    }

    unique_ptr<iColumn> copy(){
        Column<type>* p = new Column<type>(*(type*)value);
        *p = *this;
        return unique_ptr<iColumn>(p);
    }
};

template<>
struct Column<string> : iColumn
{

    Column(string& str){
        value = &str;
    }

    string getValue() const {
        return *(string*)value;
    }
    void setValue(const char* str){
        *(string*)value = str;
    }
    bool isNull() const{
        return ((string*)value)->empty();
    }
    unique_ptr<iColumn> copy(){
        Column<string>* p = new Column<string>(*(string*)value);
        *p = *this;
        return unique_ptr<iColumn>(p);
    }
};

template<>
struct Column<vector<int>> : iColumn
{
    Column(vector<int>& vector){
        value = &vector;
    }

    string getValue() const {
        string str;
        for(int i: *((vector<int>*)value)){
            if(str.size())
                str+=',';
            str+= to_string(i);
        }
        return str;
    }
    void setValue(const char* str){
        char *token = std::strtok((char*)str, ",");
        while (token != NULL) {
            ((vector<int>*)value)->emplace_back(stoi(token));
            token = std::strtok(NULL, ",");
        }
    }
    bool isNull() const{
        return ((vector<int>*)value)->empty();
    }
    unique_ptr<iColumn> copy(){
        Column<vector<int>>* p = new Column<vector<int>>(*(vector<int>*)value);
        *p = *this;
        return unique_ptr<iColumn>(p);
    }
};

typedef chrono::time_point<std::chrono::system_clock> date_time;
template<>
struct Column<date_time> : iColumn
{

    Column(date_time& date) {
        value = &date;
    }

    string getValue() const {
        std::time_t tp= chrono::system_clock::to_time_t( *(date_time*)value );
        return std::asctime(std::gmtime(&tp));
    }
    void setValue(const char* str){
        struct std::tm tm;
        std::istringstream ss(str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H-%M-%S"); // or just %T in this case
        std::time_t time = mktime(&tm);
        *(date_time*)value = chrono::system_clock::from_time_t(time);
    }
    bool isNull() const {
        return *(date_time*)value==chrono::time_point<std::chrono::system_clock>{};
    }
    unique_ptr<iColumn> copy(){
        Column<date_time>* p = new Column<date_time>(*(date_time*)value);
        *p = *this;
        return unique_ptr<iColumn>(p);
    }
};

struct ForeignKey{
    iColumn* column;
    string reference;
};

template<class type>
struct Table
{
    static string table_name;
    vector<unique_ptr<iColumn>> columns;  //  coluna: name, type
    vector<ForeignKey> vecFK;

    Table(const string& table = tolower_str(string(typeid(type).name()+1)) ){
        table_name = table;
    }

    Table(Table&& hrs){}
    Table(const Table& hrs){
        type* objThis = (type*)this;
        type * objHrs = (type*)&hrs;

        for(auto&& col: hrs.columns){
            uintptr_t pcol = reinterpret_cast<uintptr_t>(col->value);
            uintptr_t phrs = reinterpret_cast<uintptr_t>(objHrs);
            uintptr_t pthis = reinterpret_cast<uintptr_t>(objThis);
            void* pvalue = reinterpret_cast<void*>((pcol-phrs)+pthis);

            this->columns.emplace_back(col->copy());
            auto&& copy = columns.back();
            copy->value = pvalue;
        }

    }
    void operator =(const Table& hrs){}

    template<class tVar>
    iColumn* column(tVar& var, string name, PropertyColumn prop=Data, string type_db = getTypeDB(typeid(tVar)) ){
        columns.emplace_back(new Column<tVar>(var));
        auto&& col = columns.back();
        col->setProperties(name, type_db, prop);
        return col.get();
    }

    void foreignKey(iColumn* column, string reference){
        vecFK.emplace_back(ForeignKey{column, reference});
    }

    string sql_create(vector<unique_ptr<iColumn>>& columns){
        short qtdPK=0;
        string sql_create = "CREATE TABLE IF NOT EXISTS "+table_name+"(\n";
        for(int i=0; i<columns.size(); i++){
            iColumn* col = columns[i].get();
            sql_create += col->name +' '+col->type_db;
            sql_create += (col->prop==NotNull)?" NOT NULL ":"";
            if(i<columns.size()-1)
                sql_create+=",\n";
            if(col->prop == PrimaryKey)
                qtdPK++;
        }
        if(qtdPK){
            sql_create += ", CONSTRAINT PK_"+table_name+" PRIMARY KEY (";
            for(int i=0; i<columns.size(); i++){
                iColumn* col = columns[i].get();
                if(col->prop == PrimaryKey){
                    sql_create += col->name;
                    if(i<qtdPK-1)
                        sql_create += ", ";
                }
            }
            sql_create += ")\n";
        }
        if(vecFK.size()){
            for(int i=0; i<vecFK.size(); i++){
                sql_create += ", CONSTRAINT FK_"+vecFK[i].column->name+table_name+
                    " FOREIGN KEY ("+vecFK[i].column->name+") REFERENCES "+vecFK[i].reference;
            }
        }
        sql_create += "\n);\n";
        for(auto& col: columns)
            if(col->prop == Index)
                sql_create += "CREATE INDEX IF NOT EXISTS idx_"+col->name+" ON "+table_name+'('+col->name+");\n";

        return sql_create;
    }
};

template<class type>
string Table<type>::table_name;

#endif // ORM_H
