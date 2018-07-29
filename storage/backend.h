#ifndef BACKEND_H
#define BACKEND_H

#include <memory>
#include <functional>
#include "mor/entity.h"
#include <boost/algorithm/string/replace.hpp>

namespace storage
{

using namespace std;

enum Comparator {
    EQUAL,
    DIFFERENT,
    BIGGER_THEN,
    LESS_THAN,
    LIKE,
    ARRAY_JSON,
};

struct Condition {
    const string& col;
    Comparator comparator;
    string val;

    Condition(const string& str)
        : col{str} {
    }

    template<class type>
    Condition operator==( type arg){
        this->comparator = Comparator::EQUAL;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator!=( type arg){
        this->comparator = Comparator::DIFFERENT;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator>( type arg){
        this->comparator = Comparator::BIGGER_THEN;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator<( type arg){
        this->comparator = Comparator::LESS_THAN;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator % ( type arg){
        this->comparator = Comparator::LIKE;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }

    template<class type>
    Condition operator |( type arg){
        this->comparator = Comparator::ARRAY_JSON;
        stringstream ss; ss<< arg; ss >> this->val;
        return *this;
    }
};

//template<typename type>

//template<class type>
//Condition condition(string col, Comparator comparator, type val){
//    string str; stringstream ss; ss<<val; ss >> str;
//    return Condition{col, comparator, str};
//}

enum Operator {
    AND,
    OR
};


bool isPrimaryKey(const mor::DescField& desc);

bool isIndexed(const mor::DescField& desc);

string to_string(const Operator& ope);

string to_string(const Condition &condition);

string getTypeDB(const mor::DescField &desc, const shared_ptr<mor::iField>& fi);

template <typename T>
string where(const T& t) {
    string str = to_string(t);
    return str;
}

template <typename First, typename... Rest>
string where(const First& first, const Rest&... rest) {
    string str = to_string(first);
    str += where(rest...); // recursive call using pack expansion syntax
    return str;
}

string sql_create(const string& table_name, vector<mor::DescField> &descs, const vector<shared_ptr<mor::iField>>& fields);

template<typename TypeBackend>
class Backend
{
    static shared_ptr<TypeBackend> instance;
public:
    Backend(){}

    virtual string exec_sql(const string& sql, const vector<shared_ptr<mor::iField>>& columns={}, const vector<mor::DescField>& descs={}) const = 0;

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

    template<class type>
    void remove(const string& where) const{
        type obj;
        mor::Entity<type>* table = (mor::Entity<type>*) &obj;
        string sql = "delete from "+table->_entity_name+" where "+where;

        exec_sql(sql);
    }

    /**
     * @brief update
     * @param bean, Objeto os valores a serem atualizado
     * @param where, string com os parametros de busca dos objetos a serem atualizados.
     * @param collumns, as colunas que seram atualizadas, caso nenuma seja inforamda todas as colunas seram atualizadas
     */
    template<class type>
    string update(type& bean, const string& where, const vector<const char*>& collumns) const
    {
        mor::Entity<type>* entity = (mor::Entity<type>*) &bean;
        const string& sql = (getSqlUpdate(entity, collumns)+" where "+where);

        return exec_sql(sql);
    }

    /**
     * @brief update
     * @param bean, Objeto os valores a serem atualizado
     * @param where, string com os parametros de busca dos objetos a serem atualizados.
     * @param colls..., as colunas que seram atualizadas, caso nenuma seja inforamda todas as colunas seram atualizadas
     */
    template<class type, typename... Args>
    string update(type& bean, const string& where, const Args&... colls) const
    {
        std::initializer_list<const char*> inputs({colls...});
        vector<const char*> collumns(inputs);

        return update(bean, where, collumns);
    }

    template<class type>
    void insert(type& bean) const{
        mor::Entity<type>* table = (mor::Entity<type>*) &bean;
        const string& sql = getSqlInsert(table->_entity_name, table->_fields, table->_desc_fields);

        exec_sql(sql, table->_fields, table->_desc_fields);
    }

    template<class TypeObj, class TypeRet=vector<TypeObj>>
    TypeRet find(const string& where="") const
    {
        static_assert(std::is_base_of<mor::Entity<TypeObj>, TypeObj>::value, "TypeRet is not a list<bean> valid");

        string sql = "select * from "+mor::Entity<TypeObj>::_entity_name+(where.empty()?"":" where "+where);

        return exec_sql<TypeRet>(sql);
    }

    template<class TypeObj>
    void find(const string& where, auto func) const
    {
        static_assert(std::is_base_of<mor::Entity<TypeObj>, TypeObj>::value, "TypeRet is not a list<bean> valid");

        string sql = "select * from "+mor::Entity<TypeObj>::_entity_name+(where.empty()?"":" where "+where);

        return exec_sql<TypeObj>(sql, func);
    }

    static TypeBackend& getInstance()
    {
        if(instance == nullptr){
            instance = shared_ptr<TypeBackend>(new TypeBackend());
        }
        return *instance;
    }

    virtual string getSqlInsert(const string& entity_name, vector<shared_ptr<mor::iField> >& columns, vector<mor::DescField>& descs) const
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

            sql << '\'' << val << '\'';
            if(i<columns.size()-1)
                sql << ", ";
        }
        sql << ')';
        return sql.str();
    }

    template< class type>
    string getSqlUpdate(mor::Entity<type>* entity, const vector<const char*>& collumns_to_set) const
    {
        std::stringstream sql;
        sql << "UPDATE " << entity->_entity_name << " SET ";
        if(collumns_to_set.empty()){
            for(int i=0; i<entity->_fields.size(); i++){
                const mor::iField* col = entity->_fields[i].get();

                string&& val = col->getValue(entity->_desc_fields[i]);
                boost::replace_all(val, "'", "''");

                sql << entity->_desc_fields[i].name << "=\'" << val << '\'';
                if(i<entity->_fields.size()-1)
                    sql << ", ";
            }
        }else{
            for(int j=0; j<collumns_to_set.size(); j++){
                for(int i=0; i<entity->_fields.size(); i++){
                    if(collumns_to_set[j] == entity->_desc_fields[i].name){
                        const mor::iField* col = entity->_fields[i].get();

                        string&& val = col->getValue(entity->_desc_fields[i]);
                        boost::replace_all(val, "'", "''");

                        sql << entity->_desc_fields[i].name << "=\'" << val << '\'';
                        if(j<collumns_to_set.size()-1)
                            sql << ", ";
                    }
                }
            }
        }
        return sql.str();
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
