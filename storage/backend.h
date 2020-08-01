#ifndef BACKEND_H
#define BACKEND_H

#include <memory>
#include <functional>
#include <boost/algorithm/string/replace.hpp>
#include "query.h"
#include <chrono>
#include "functions.h"
#include <mor/mor.h>
#include <d3util/datetime.h>
#include <memory>

namespace storage
{
//using namespace std;

/*
bool isPrimaryKey(iannotations* annotations);

bool isIndexed(iannotations* annotations);

string createSqlSelect(iannotations* annotations);

string getSqlInsertImpl(iannotations* annotations);

string getSqlUpdate(iannotations* annotations, const vector<const char*>& collumns_to_set);
*/

template<class T>
bool isNull(T val){
    return !val;
}

bool isNull(std::string str);

template<typename TypeBackend>
class Backend
{
    static std::shared_ptr<TypeBackend> instance;
public:
    Backend(){}

    static TypeBackend& getInstance()
    {
        if(instance == nullptr){
            instance = std::shared_ptr<TypeBackend>(new TypeBackend());
        }
        return *instance;
    }

    string exec_sql(const string& sql) const
    {
        return ((TypeBackend*)this)->exec_sql(sql);
    }

    template<class T>
    string exec_sql(const string& sql, T* entity) const
    {
        return ((TypeBackend*)this)->template exec_sql(sql, entity);
    }

    template<class TypeRet>
    TypeRet exec_sql(const string& sql) const
    {
        return ((TypeBackend*)this)->template exec_sql<TypeRet>(sql);
    }

    template<class TypeBean>
    void exec_sql(const string& sql, std::function<void(TypeBean&)> callback) const
    {        
        return ((TypeBackend*)this)->template exec_sql<TypeBean>(sql, callback);
    }

    template<class TypeObj>
    Query<TypeObj, Backend<TypeBackend>> exec(const string& sql, std::function<void(TypeObj& entity)> callback=nullptr) const
    {
        Query<TypeObj, Backend<TypeBackend>> q;
        q.db = this;
        q.sql = sql;
        q.callback = callback;
        return q;
    }

    template<class type>
    void create() const{
        type obj;
        const string& sql = sql_create(obj);
        this->exec_sql(sql);
    }

    template<class TypeObj>
    Query<TypeObj, Backend<TypeBackend>> select(std::function<void(TypeObj& entity)> callback=nullptr) const
    {
        Query<TypeObj, Backend<TypeBackend>> q;
        q.db = this;
        q.sql = getSqlSelect<TypeObj>();
        q.callback = callback;
        return q;
    }

    template<class type>
    Query<type, Backend<TypeBackend>>& update_cols(type& bean, const std::set<std::string>& colsToSet, Query<type, Backend<TypeBackend>>&& q={}) const
    {
        std::stringstream sql;
        sql << "UPDATE " << ((Entity*)type::annotations::get_entity())->name<< " SET ";

        reflector::visit_each(bean, update_each{sql, colsToSet});

        q.db = this;
        q.sql = sql.str();
        return q;
    }

    template<class type>
    Query<type, Backend<TypeBackend>>& update(type& bean, Query<type, Backend<TypeBackend>>&& q={}) const
    {
        std::stringstream sql;
        sql << "UPDATE " << ((Entity*)type::annotations::get_entity())->name<< " SET ";

        reflector::visit_each(bean, update_each{sql, {} });

        q.db = this;
        q.sql = sql.str();
        return q;
    }

    template<class type>
    void insert(type& bean) const{
        const string& sql = ((TypeBackend*)this)->getSqlInsert(bean);
        exec_sql(sql, &bean);
    }

    template<class type>
    Query<type, Backend<TypeBackend>>& remove(Query<type, Backend<TypeBackend>>&& q={}) const {
        q.db = this;
        q.sql = "DELETE FROM "+((Entity*)type::annotations::get_entity())->name;
        return q;
    }

    template<class type>
    void drop() const{
      string sql = "DROP TABLE IF EXISTS "+((Entity*)type::annotations::get_entity())->name;
      exec_sql(sql);
    }

    template<class T>
    struct GetValueStr
    {
        const T& value;
        Reference* ref{};
        string str;

        template <class V>
        auto get(const V& value) noexcept -> std::enable_if_t<is_simple_or_datatime_type<V>::value>
        {
            str = to_string(value);
        }

        template <class V>
        auto get(const V& value) noexcept -> std::enable_if_t<!is_simple_or_datatime_type<V>::value>
        {
            if(ref){
                reflector::visit_each(value, *this);
            }
        }

        template<class FieldData, class Annotations>
        void operator()(FieldData f, Annotations a, int lenght)
        {
            if(ref->field == f.name()){
                auto val = f.get();
                get(val);
            }
        }

        operator string (){
            get(value);
            return str;
        }
    };


    struct update_each
    {
        int i=0;
        std::stringstream& sql;
        const std::set<std::string>& colsToSet;
        
        update_each(std::stringstream& sql, const std::set<std::string>& colsToSet) 
            : sql(sql)
            , colsToSet(colsToSet) {
        }

        template<class FieldData, class Annotations>
        void operator()(FieldData f, Annotations a, int lenght)
        {
            Reference* ref = f.annotation();
            string val = GetValueStr<typename FieldData::type>{f.get(), ref};
            const char* name = f.name();

            if(colsToSet.size() && colsToSet.find(name)==colsToSet.end())
                return;

            if( ((PrimaryKey*)f.annotation())!=nullptr && (val.empty() || val=="0") )
                return;

            if(i++!=0)
                sql << ", ";

            Type* type = f.annotation();
            Insert* insert = f.annotation();

            if(ref && (val.empty() || val=="0"))
                sql << name << "=NULL";
            else if(type && type->name == "tsvector" && !val.empty())
                sql << name << '=' << val;
            else if(type && type->name == "GEOGRAPHY(POINT,4326)" && val.empty())
                sql << name << "=NULL";
            else if(insert)
                sql << name << '=' << insert->converter(val);
            else{
                boost::replace_all(val, "'", "''");
                sql << name << "=\'" << val << '\'';
            }
        }
    };

    struct insert_each
    {
        std::string columns;
        std::string values;

        template<class FieldData, class Annotations>
        void operator()(FieldData f, Annotations a, int lenght)
        {
            Reference* ref = f.annotation();
            string val = GetValueStr<typename FieldData::type>{f.get(), ref};

            if( ((PrimaryKey*)f.annotation())!=nullptr
                    && (val.empty() || val =="0") )
                return;

            if(columns.size())
                columns += ", ";
            columns += f.name();

            if(values.size())
                values += ", ";

            Type* type = f.annotation();
            Insert* insert = f.annotation();

            if(ref && (val.empty() || val =="0")){
                values += "NULL";
                return;
            }else if(insert){
                values += insert->converter(val);
                return;
            }else if(type){
                if(type->name == "GEOGRAPHY(POINT,4326)" && val.empty()){
                    values += "NULL";
                    return;
                }else if(type->name == "tsvector" && !val.empty()){
                    values += val;
                    return;
                }
            }

            boost::replace_all(val, "'", "''");
            values += '\'' + val + '\'';
        }
    };

    template<class T>
    string getSqlInsertBase(T& bean) const
    {
        insert_each ie;
        reflector::visit_each(bean, ie);
        std::string sql = "INSERT INTO " + ((Entity*)T::annotations::get_entity())->name + '(';
        sql += ie.columns;
        sql += ") VALUES (" +ie.values+ ')';
        return sql;
    }

    template<class T>
    struct getListPK
    {
        T& obj;
        string pks;

        getListPK(T& obj) : obj(obj){}

        template<class FieldData, class Annotations>
        void operator()(FieldData f, Annotations a, int lenght)
        {
            if(((PrimaryKey*)f.annotation())){
                if(pks.size())
                    pks += ", ";
                pks += f.name();
            }
        }

        operator string (){
            reflector::visit_each(obj, *this);
            return pks;
        }
    };
};

template<typename TypeBackend>
std::shared_ptr<TypeBackend> Backend<TypeBackend>::instance;
}
#endif // BACKEND_H


