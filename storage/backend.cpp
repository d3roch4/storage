#include "backend.h"
#include <sstream>
#include <mor/ientity.h>
#include <set>

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
        return "bit(1)";
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
        sql_create += ")";
        for(DescField& desc: descs){
            auto ref = desc.options.find("reference");
            if(ref != desc.options.end()){
                auto fie = desc.options.find("field");
                if(fie==desc.options.end())
                    throw_with_trace(runtime_error("field option not found in entity: "+table_name));
                else
                    sql_create += ",\nFOREIGN KEY ("+desc.name+") REFERENCES "+ref->second+"("+fie->second+")";
            }
        }
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

#if DEBUG
    clog << __PRETTY_FUNCTION__ << sql_create << endl;
#endif
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

void putJoinsSelect(mor::iEntity *entity, string& sql, set<string>& joinsRelations)
{
    auto&& vecDesc = entity->_get_desc_fields();
    for(int i=0; i<vecDesc.size(); i++){
        mor::DescField& desc = vecDesc[i];
        auto&& ref = desc.options.find("reference");
        if(ref != desc.options.end() && joinsRelations.find(ref->second)==joinsRelations.end()){
            const string& relation = ref->second;
            sql += "\nLEFT JOIN "+relation+" ON "+entity->_get_name()+'.'+desc.name+'='+ref->second+'.'+desc.options["field"];
            joinsRelations.insert(relation);
            putJoinsSelect((mor::iEntity*)entity->_get_fields()[i]->value, sql, joinsRelations);
        }
    }
}

std::string createSqlSelect(mor::iEntity* entity)
{
    set<string> joinsRelations;
    string sql = "SELECT ";
    putCollumnsSelect(entity, sql);
    sql += "\nFROM "+entity->_get_name();
    putJoinsSelect(entity, sql, joinsRelations);
    entity->_get_atrributes()["sql_select"] = sql;
    return sql;
}

string getSqlInsertImpl(const string &entity_name, vector<shared_ptr<iField> > &columns, vector<DescField> &descs)
{
    std::stringstream sql;
    sql << "INSERT INTO " << entity_name << '(';
    for(int i=0; i<columns.size(); i++){
        const mor::iField* col = columns.at(i).get();
        if(isPrimaryKey(descs[i]) && col->isNull())
            continue;
        sql << descs[i].name;
        if(i<columns.size()-1)
            sql << ", ";
    }
    sql << ") VALUES (";
    for(int i=0; i<columns.size(); i++){
        const mor::iField* col = columns.at(i).get();
        if(isPrimaryKey(descs[i]) && col->isNull())
            continue;

        string&& val = col->getValue(descs[i]);
        boost::replace_all(val, "'", "''");

        auto ref = descs[i].options.find("reference");
        if(ref != descs[i].options.end() && (val.empty() || val =="0"))
            sql << "NULL";
        else
            sql << '\'' << val << '\'';
        if(i<columns.size()-1)
            sql << ", ";
    }
    sql << ')';
    return sql.str();
}

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


}
