#include "backend.h"
#include <sstream>
#include <mor/ientity.h>

using namespace mor;

namespace storage
{

string getTypeDB(const DescField& desc, const shared_ptr<iField>& fi)
{
    const type_index &ti = *desc.typeinfo;

    if(ti == typeid(string))
        return "text";
    else if(ti == typeid(int) || ti==typeid(unsigned int))
        return "int";
    else if(ti == typeid(char) || ti==typeid(unsigned char))
        return "char";
    else if(ti == typeid(long) || ti==typeid(unsigned long))
        return "bigint";
    else if(ti == typeid(short) || ti==typeid(unsigned short))
        return "smallint";
    else if(ti == typeid(bool))
        return "bit";
    else if(ti == typeid(float))
        return "float";
    else if(ti == typeid(double))
        return "float";
    else if(ti == typeid(chrono::time_point<std::chrono::system_clock>))
        return "TIMESTAMP";
    else{
        auto ref = desc.options.find("reference");
        if(ref != desc.options.end()){
            iEntity* ptr = (iEntity*) fi->value;
            const vector<DescField>& descsChild = ptr->_get_desc_fields();
            const vector<shared_ptr<iField>>& fieldsChild = ptr->_get_fields();
            const string& refName = desc.options.find("field")->second;
            for(int i=0; i<descsChild.size(); i++)
                if(descsChild[i].name == refName)
                    return getTypeDB(descsChild[i], fieldsChild[i]);
        }
    }

    throw_with_trace(runtime_error(string("getTypeDB: type not implemented for: ")+ti.name()));
}

bool isPrimaryKey(const DescField& desc)
{
    for(const pair<string, string>& opt: desc.options){
        if(opt.first == "attrib" && opt.second == "PK")
            return true;
    }
    return false;
}

bool isIndexed(const DescField& desc)
{
    for(const pair<string, string>& opt: desc.options){
        if(opt.first == "attrib" && opt.second == "INDEXED")
            return true;
    }
    return false;
}

string sql_create(const string &table_name, vector<DescField>& descs, const vector<shared_ptr<mor::iField>>& fields){
    short qtdPK=0;
    string sql_create = "CREATE TABLE IF NOT EXISTS "+table_name+"(\n";
    for(int i=0; i<descs.size(); i++)
    {
        if(descs[i].options.find("type_db")==descs[i].options.end())
            descs[i].options["type_db"] = getTypeDB(descs[i], fields[i]);
        sql_create += descs[i].name +' '+descs[i].options["type_db"];
        sql_create += (descs[i].options["attrib"]=="NotNull")?" NOT NULL ":"";
        if(i<descs.size()-1)
            sql_create+=",\n";
        if(isPrimaryKey(descs[i]))
            qtdPK++;
    }
    if(qtdPK){
        sql_create += ", CONSTRAINT PK_"+table_name+" PRIMARY KEY (";
        for(int i=0; i<descs.size(); i++){
            if(isPrimaryKey(descs[i])){
                sql_create += descs[i].name;
                if(i<qtdPK-1)
                    sql_create += ", ";
            }
        }
        sql_create += ")\n";
    }
    /*if(table->_vecFK.size()){
        for(int i=0; i<table->_vecFK.size(); i++){
            sql_create += ", CONSTRAINT FK_"+table->_vecFK[i].field->name+table->_entity_name+
                " FOREIGN KEY ("+table->_vecFK[i].field->name+") REFERENCES "+table->_vecFK[i].reference;
        }
    }*/
    sql_create += "\n);\n";
    for(const auto& col: descs)
        if(isIndexed(col))
            sql_create += "CREATE INDEX IF NOT EXISTS idx_"+col.name+" ON "+table_name+'('+col.name+");\n";

    return sql_create;
}

void putCollumnsSelect(mor::iEntity *entity, string& sql)
{
    sql += entity->_get_name()+".* ";
    auto&& vecDesc = entity->_get_desc_fields();
    for(int i=0; i<vecDesc.size(); i++){
        mor::DescField& desc = vecDesc[i];
        auto&& ref = desc.options.find("reference");
        if(ref != desc.options.end()){
            sql += ", ";
            putCollumnsSelect((iEntity*)entity->_get_fields()[i]->value, sql);
        }
    }
}

void putJoinsSelect(mor::iEntity *entity, string& sql)
{
    auto&& vecDesc = entity->_get_desc_fields();
    for(int i=0; i<vecDesc.size(); i++){
        mor::DescField& desc = vecDesc[i];
        auto&& ref = desc.options.find("reference");
        if(ref != desc.options.end()){
            sql += "\nLEFT JOIN "+ref->second+" ON "+entity->_get_name()+'.'+desc.name+'='+ref->second+'.'+desc.options["field"];
            putJoinsSelect((mor::iEntity*)entity->_get_fields()[i]->value, sql);
        }
    }
}

std::string createSqlSelect(mor::iEntity* entity)
{
    string sql = "SELECT ";
    putCollumnsSelect(entity, sql);
    sql += "\nFROM "+entity->_get_name();
    putJoinsSelect(entity, sql);
    entity->_get_atrributes()["sql_select"] = sql;
    return sql;
}


}
