#ifndef QUERY_H
#define QUERY_H
#include <sstream>
#include <vector>
#include <mor/entity.h>

template<class TypeEntity, class TypeBackend>
struct Query {
    const TypeBackend* db;
    std::string sql;
    mutable bool execulted=false;

    Query(){ }

    ~Query(){
        if(!execulted)
            db->exec_sql(sql);
    }

    operator TypeEntity() const {
        TypeEntity obj;
        execulted=true;
        db->exec_sql(sql+" LIMIT 1", &obj);
        return obj;
    }

    operator std::vector<TypeEntity>() const {
        execulted=true;
        return db->template exec_sql< std::vector<TypeEntity> >(sql);
    }

    Query<TypeEntity, TypeBackend>& where(){
        sql += " WHERE ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& and_(){
        sql += " AND ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& like(const std::string& column, const std::string& value){
        sql += column + " LIKE '" + value + "' ";
        return *this;
    }

    Query<TypeEntity, TypeBackend>& eq(const std::string& column, auto value){
        std::stringstream ss; ss << value;
        sql += column + "='" +ss.str() + "' ";
        return *this;
    }
};


#endif // QUERY_H
