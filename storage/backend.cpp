#include "backend.h"
#include <sstream>
#include <mor/mor.h>
#include <set>
#include <d3util/stacktrace.h>


namespace storage
{

struct insertColumns{
    string names;
    string values;
    int i=0;

    template<class FieldData, class Annotations>
    void operator()(FieldData f, Annotations a, int lenght)
    {
        if(((PrimaryKey*)f.anntation()) && isNull(f.get()))
            return;

        names += f.name();
        if(i<lenght-1)
            names += ", ";

        string&& val = to_string(f.get());
        boost::replace_all(val, "'", "''");\
        Reference* ref = f.annotation();
        if(ref && (val.empty() || val =="0"))
            values += "NULL";
        else
            values += '\'' + val + '\'';
        if(i<lenght-1)
            values += ", ";
    }
};

template<class T>
string getSqlInsertImpl(T& obj)
{
    std::stringstream sql;
    Entity* entity = T::annotations::get_entity();
    insertColumns ic;
    reflector::visit_each(obj, ic);
    sql << "INSERT INTO " << entity->name << '(';
    sql << ic.names;
    sql << ") VALUES (";
    sql << ic.values;
    sql << ')';
    return sql.str();
}

bool isNull(string str){
    return str.empty();
}

/*
string getSqlUpdate(iEntity *entity, const vector<const char*>& collumns_to_set)
{
    std::stringstream sql;
    const auto& descs = entity->_get_desc_fields();
    const auto& fields = entity->_get_fields();

    sql << "UPDATE " << entity->_get_name()<< " SET ";
    if(collumns_to_set.empty()){
        for(int i=0; i<fields.size(); i++){
            const mor::iField* col = fields[i].get();

            if(isPrimaryKey(descs[i]) && col->isNull())
                continue;

            string&& val = col->getValue(descs[i]);
            boost::replace_all(val, "'", "''");

            auto ref = descs[i].options.find("reference");
            if(ref != descs[i].options.end() && (val.empty() || val=="0"))
                sql << descs[i].name << "=NULL";
            else
                sql << descs[i].name << "=\'" << val << '\'';

            if(i<fields.size()-1)
                sql << ", ";
        }
    }else{
        for(int j=0; j<collumns_to_set.size(); j++){
            for(int i=0; i<fields.size(); i++){
                if(collumns_to_set[j] == descs[i].name){
                    const mor::iField* col = fields[i].get();

                    string&& val = col->getValue(descs[i]);
                    boost::replace_all(val, "'", "''");

                    auto ref = descs[i].options.find("reference");
                    if(ref != descs[i].options.end() && (val.empty() || val=="0"))
                        sql << descs[i].name << "=NULL";
                    else
                        sql << descs[i].name << "=\'" << val << '\'';

                    if(j<collumns_to_set.size()-1)
                        sql << ", ";
                }
            }
        }
    }
    return sql.str();
}
*/

}
