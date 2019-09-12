#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "annotations.h"
#include <mor/mor.h>
#include <d3util/stacktrace.h>
#include <exception>
#include <set>
#include <d3util/datetime.h>

namespace storage
{
using namespace std;

struct eachputCollumnsSelect
{
    string cols;
    string joins;
    std::set<string> joinsRelations;
    string aliasTable;

    template<class FieldData, class Annotations>
    auto operator()(FieldData f, Annotations a, int lenght) noexcept -> std::enable_if_t<
                    is_simple_or_datatime_type<typename FieldData::type>::value >
    {
        if(cols.size())
            cols += ", ";
        Entity* ent = a.get_entity();
        if(aliasTable.empty())
            cols += ent->name+'.'+f.name();
        else
            cols += aliasTable+'.'+f.name();
    }

    template<class FieldData, class Annotations>
    auto operator()(FieldData f, Annotations a, int lenght) noexcept -> std::enable_if_t<
                            !is_simple_or_datatime_type<typename FieldData::type>::value >
    {
        string name = f.name();
        Reference* ref = Annotations::get_field(name.c_str());
        if(ref){
            auto&& val = f.get();
            Entity* entity = Annotations::get_entity();

            string aliasTemp = aliasTable;
            aliasTable += (aliasTable.empty()?name:'_'+name);

            joins += "\nLEFT JOIN "+ref->entity+" AS "+aliasTable
                    +" ON "+aliasTable+'.'+ref->field
                    +'='+(aliasTemp.empty()?entity->name:aliasTemp)+'.'+name;
            joinsRelations.insert(ref->entity);

            reflector::visit_each(val, *this);
            aliasTable = aliasTemp;
        }
    }
};

struct putJoinsSelect{
    string& sql;
    std::set<string>& joinsRelations;
    putJoinsSelect(string& sql, set<string>& joinsRelations) :
        sql(sql),
        joinsRelations(joinsRelations) {}

    template<class FieldData, class Annotations>
    void operator()(FieldData f, Annotations a, int lenght)
    {
        Reference* ref = f.annotation();
        if(ref && joinsRelations.find(ref->entity)==joinsRelations.end()){
            Entity* entity = Annotations::get_entity();
            sql += "\nLEFT JOIN "+ref->entity+" ON "+entity->name+'.'+f.name()+'='+ref->entity+'.'+ref->field;
            joinsRelations.insert(ref->entity);

            reflector::visit_each(f.get(), putJoinsSelect{sql, joinsRelations});
        }
    }
};

template<class T>
string createSqlSelect()
{
    set<string> joinsRelations;
    Entity* entity = T::annotations::get_entity();
    string sql = "SELECT ";
    T obj;
    eachputCollumnsSelect cs;
    reflector::visit_each(obj, cs);
    sql += cs.cols;
    sql += "\nFROM "+entity->name;
    sql += cs.joins;
    T::annotations::put_entity(Select{sql});
    return sql;
}

template<class T>
string getSqlSelect()
{
    Select* select = T::annotations::get_entity();
    if(select)
        return select->sql;
    else{
        return createSqlSelect<T>();
    }
}


template<class V, class A>
struct getTypeDB_s;
template<class V, class A>
inline string getTypeDB(const V& val, A& a, const char* name){
    string s{name};
    return getTypeDB_s<V, A>(val, a, s);
}

template<class V, class A>
struct getTypeDB_s{
    string& name;
    string type;
    const V& val;
    Reference* ref{};

    getTypeDB_s(const V& val, A& a, string& name) :
        val(val),
        name(name){
    }

    template<class FieldData, class Annotations>
    void operator()(FieldData f, Annotations a, int lenght)
    {
        if(name == f.name()){
            type = typeName(f.get());
        }
    }

    template <class T, class Dummy = void>
    auto getTypeObj(const T& val) noexcept -> std::enable_if_t<is_simple_or_datatime_type<T>::value >
    {}

    template <class T, class Dummy = void>
    auto getTypeObj(const T& val) noexcept -> std::enable_if_t<!is_simple_or_datatime_type<T>::value >
    {
        ref = A::get_field(name.c_str());
        if(ref){
            name = ref->field;
            reflector::visit_each(val, *this);
        }
    }

    template<class T>
    string typeName(const T& value){
        const type_info &ti = typeid(T);
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
        else if(ti == typeid(std::chrono::time_point<std::chrono::system_clock>))
            return "TIMESTAMP";
        else{
            getTypeObj(value);
            if(ref){
                return this->type;
            }
        }

        throw_with_trace(runtime_error("getTypeDB: type not implemented for: "+name));
        return "";
    }

    operator string () {
        return typeName(val);
    }
};

struct eachCreate{
    string columns;
    string constraints;
    string references;
    string indexes;
    int i=0;

    template<class FieldData, class Annotations>
    void operator()(FieldData f, Annotations a, int lenght)
    {
        const string& name = f.name();
        Type* type = Annotations::get_field(name.c_str());

        columns += name; columns += ' ';
        if(type)
            columns += type->name;
        else{
            const auto& v = f.get();
            columns += getTypeDB(v, a, name.c_str());
        }

        if(((NotNull*) Annotations::get_field(name.c_str())))
            columns += " NOT NULL ";
        if(i<lenght-1)
            columns+=",\n";

        if((PrimaryKey*) Annotations::get_field(name.c_str())){
            constraints += name;
            constraints += ", ";
        }

        Reference* ref =  Annotations::get_field(name.c_str());
        if(ref){
            if(ref->field.empty()){
                const string s = "field option not found in entity: "+((Entity*)Annotations::get_entity())->name;
                throw_with_trace(runtime_error(s));
            }else{
                references += ",\nFOREIGN KEY (";
                references += name;
                references += ") REFERENCES "+ref->entity+"("+ref->field+")";
            }
        }

        Indexed* idx = Annotations::get_field(name.c_str());
        if(idx){
            const string& entity  = ((Entity*)Annotations::get_entity())->name;
            indexes += "CREATE INDEX IF NOT EXISTS idx_"+entity+'_';
            indexes += name;
            indexes += " ON "+entity+ 
                (idx->using_.empty() ? '('+name+");\n" : " USING "+idx->using_);
        }

        i++;
    }
};


template<class T>
string sql_create(T& obj){
    Entity* entity = T::annotations::get_entity();
    string sql_create = "CREATE TABLE IF NOT EXISTS "+entity->name+"(\n";
    eachCreate ec;
    reflector::visit_each(obj, ec);
    ec.constraints = ec.constraints.substr(0, ec.constraints.size()-2);
    sql_create += ec.columns;
    if(ec.constraints.size()){
        sql_create += ", CONSTRAINT PK_"+entity->name+" PRIMARY KEY (";
        sql_create += ec.constraints;
        sql_create += ")";
    }
    sql_create += ec.references;
    sql_create += "\n);\n";
    sql_create += ec.indexes;

#if DEBUG
    clog << __PRETTY_FUNCTION__ << sql_create << endl;
#endif
    return sql_create;
}


}
#endif // FUNCTIONS_H
