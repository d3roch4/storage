#ifndef QUERY_H
#define QUERY_H
#include "expression.h"
#include <sstream>
#include <list>
#include <functional>
#include <vector>

template<class TypeEntity, class TypeBackend>
struct Query {
    const TypeBackend* db;
    std::string sql;
    mutable bool execulted=false;
    std::function<void(TypeEntity& entity)> callback = nullptr;

    Query(){ }

    ~Query() noexcept(false) {
        if(!execulted){
            if(callback != nullptr)
                db->template exec_sql<TypeEntity>(sql, callback);
            else
                db->exec_sql(sql);

            execulted=true;
        }
    }

    operator TypeEntity() const {
        TypeEntity obj;
        execulted=true;
        db->exec_sql(sql+" LIMIT 1", &obj);
        return obj;
    }

    operator std::list<TypeEntity>() const {
        execulted=true;
        return db->template exec_sql< std::list<TypeEntity> >(sql);
    }

    Query<TypeEntity, TypeBackend>& where(){
        sql += " WHERE ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& where(const std::string& expression){
        if(expression.size())
            sql += " WHERE "+expression;
        return *this;
    }

    template<class V>
    Query<TypeEntity, TypeBackend>& eq(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        sql += column + "='" +ss.str() + "' ";
        return *this;
    }

    template<class V>
    Query<TypeEntity, TypeBackend>& gt(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        sql += column + ">'" +ss.str() + "' ";
        return *this;
    }

    template<class V>
    Query<TypeEntity, TypeBackend>& ge(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        sql += column + ">='" +ss.str() + "' ";
        return *this;
    }

    template<class V>
    Query<TypeEntity, TypeBackend>& lt(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        sql += column + "<'" +ss.str() + "' ";
        return *this;
    }

    template<class V>
    Query<TypeEntity, TypeBackend>& le(const std::string& column, const V& value){
        std::stringstream ss; ss << value;
        sql += column + "<='" +ss.str() + "' ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& and_(){
        sql += " AND ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& or_(){
        sql += " OR ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& like(const std::string &column, const std::string &value){
        sql += column + " LIKE '" + value + "' ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& in(const std::string &column, const std::string &value){
        sql += column + " IN ("+value+") ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& not_in(const std::string &column, const std::string &value){
        sql += column + " NOT IN ("+value+") ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& not_null(const std::string &column){
        sql += column + " IS NOT NULL ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& is_null(const std::string &column){
        sql += column + " IS NULL ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& orderBy(const std::string &order){
        sql += " ORDER BY "+order;
        return *this;
    }

    template<class V>
    Query<TypeEntity, TypeBackend>& limit(const V& limit){
        std::stringstream ss; ss << limit;
        sql += " LIMIT "+ss.str();
        return *this;
    }

    template<class V>
    Query<TypeEntity, TypeBackend>& offset(const V& offset){
        std::stringstream ss; ss << offset;
        sql += " OFFSET "+ss.str();
        return *this;
    }
};


#endif // QUERY_H
