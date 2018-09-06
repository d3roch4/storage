#ifndef BACKEND_H
#define BACKEND_H

#include <memory>
#include <functional>
#include <mor/entity.h>
#include <boost/algorithm/string/replace.hpp>
#include "query.h"

namespace storage
{

using namespace std;

bool isPrimaryKey(const mor::DescField& desc);

bool isIndexed(const mor::DescField& desc);

string getTypeDB(const mor::DescField &desc, const shared_ptr<mor::iField>& fi);

string sql_create(const string& table_name, vector<mor::DescField> &descs, const vector<shared_ptr<mor::iField>>& fields);

string createSqlSelect(mor::iEntity* entity);

string getSqlInsertImpl(const string& entity_name, vector<shared_ptr<mor::iField> >& columns, vector<mor::DescField>& descs);

string getSqlUpdate(mor::iEntity* entity, const vector<const char*>& collumns_to_set);

template<typename TypeBackend>
class Backend
{
    static shared_ptr<TypeBackend> instance;
public:
    Backend(){}


    virtual string exec_sql(const string& sql, mor::iEntity* entity = 0) const = 0;

    template<class TypeRet>
    TypeRet exec_sql(const string& sql) const
    {
        return ((TypeBackend*)this)->template exec_sql<TypeRet>(sql);
    }

    template<class TypeBean>
    void exec_sql(const string& sql, std::function<void(TypeBean&)> func) const
    {
        return ((TypeBackend*)this)->template exec_sql<TypeBean>(sql, func);
    }

    template<class type>
    void create() const{
        const type obj;
        mor::Entity<type>* table = (mor::Entity<type>*) &obj;
        const string& sql = sql_create(table->_entity_name, table->_desc_fields, table->_fields);
        exec_sql(sql);
    }

    template<class type>
    void drop() const{
        const type obj;
        string sql = "DROP TABLE IF EXISTS "+mor::Entity<type>::_entity_name;

        exec_sql(sql);
    }


    template<class type> [[deprecated("Replaced by remove<TypeEntity>().where()..., which has an improved interface")]]
    void remove(const string& where) const{
        string sql = "delete from "+mor::Entity<type>::_entity_name+" where "+where;

        exec_sql(sql);
    }

    template<class type>
    Query<type, Backend<TypeBackend>>& remove(Query<type, Backend<TypeBackend>>&& q={}) const {

        q.db = this;
        q.sql = "delete from "+mor::Entity<type>::_entity_name;
        return q;
    }

    template<class type>
    Query<type, Backend<TypeBackend>>& update(type& bean, const vector<const char*>& collumns, Query<type, Backend<TypeBackend>>&& q={}) const
    {
        mor::Entity<type>* entity = (mor::Entity<type>*) &bean;
        const string& sql = getSqlUpdate(entity, collumns);

        q.db = this;
        q.sql = sql;
        return q;
    }

    /**
     * @brief update
     * @param bean, Objeto os valores a serem atualizado
     * @param where, string com os parametros de busca dos objetos a serem atualizados.
     * @param colls..., as colunas que seram atualizadas, caso nenuma seja inforamda todas as colunas seram atualizadas
     */
    template<class TypeObj, typename... Args>
    Query<TypeObj, Backend<TypeBackend>> update(TypeObj& bean, const Args&... colls) const
    {
        std::initializer_list<const char*> inputs({colls...});
        vector<const char*> collumns(inputs);
        mor::Entity<TypeObj>* entity = (mor::Entity<TypeObj>*) &bean;

        Query<TypeObj, Backend<TypeBackend>> q;
        q.db = this;
        q.sql = getSqlUpdate(entity, collumns);
        return q;
    }

    template<class type>
    void insert(type& bean) const{
        mor::Entity<type>* table = (mor::Entity<type>*) &bean;
        const string& sql = getSqlInsert(table->_entity_name, table->_fields, table->_desc_fields);

        exec_sql(sql, table);
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

    template<class TypeObj, class TypeRet=vector<TypeObj>>
    TypeRet find(const string& where="") const
    {
        static_assert(std::is_base_of<mor::Entity<TypeObj>, TypeObj>::value, "TypeRet is not a list<bean> valid");

        string sql = getSqlSelect<TypeObj>()+' '+where;

        return exec_sql<TypeRet>(sql);
    }

    template<class TypeObj>
    [[deprecated("Replaced by select<TypeEntity>().where()...")]]
    void find(const string& where, auto func) const
    {
        static_assert(std::is_base_of<mor::Entity<TypeObj>, TypeObj>::value, "TypeRet is not a list<bean> valid");

        string sql = getSqlSelect<TypeObj>();
        sql += ' '+where;

        return exec_sql<TypeObj>(sql, func);
    }

    static TypeBackend& getInstance()
    {
        if(instance == nullptr){
            instance = shared_ptr<TypeBackend>(new TypeBackend());
        }
        return *instance;
    }

    template<class T>
    string getSqlSelect() const
    {
        auto&& it = mor::Entity<T>::_atrributes.find("sql_select");
        if(it!=mor::Entity<T>::_atrributes.end())
            return boost::any_cast<string>(it->second);
        else{
            T entity;
            return createSqlSelect(&entity);
        }
    }

    virtual string getSqlInsert(const string& entity_name, vector<shared_ptr<mor::iField> >& columns, vector<mor::DescField>& descs) const
    {
        return getSqlInsertImpl(entity_name, columns, descs);
    }

    template<class type>
    string getWherePK(mor::Entity<type>* entity) const
    {
        string where;
        for(int i=0; i<entity->_fields.size(); i++){
            mor::iField* col = entity->_fields.at(i).get();
            if(isPrimaryKey(entity->_desc_fields[i]))
                where += entity->_desc_fields[i].name
                        +"='"+col->getValue(entity->_desc_fields[i])
                        +"' AND ";
        }

        if(where.size())
            where[where.size()-4] = '\0';
        return where;
    }

    string getListPK(vector<mor::DescField>& descs) const
    {
        string pks;
        for(int i=0; i<descs.size(); i++){
            if(isPrimaryKey(descs[i]))
                pks += descs[i].name + ", ";
        }
        if(pks.size())
            pks[pks.size()-2] = '\0';
        return pks;
    }


};

template<typename TypeBackend>
shared_ptr<TypeBackend> Backend<TypeBackend>::instance;

//template<typename TypeBackend>
//string Backend<TypeBackend>::connection;


}
#endif // BACKEND_H
