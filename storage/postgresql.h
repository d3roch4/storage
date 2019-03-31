#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include "backend.h"
#ifdef __APPLE__
#   include <libpq-fe.h>
#else
#   include <postgresql/libpq-fe.h>
#endif
#include <vector>
#include <type_traits>
#include <mutex>
#include <d3util/stacktrace.h>
#include <d3util/logger.h>
#include <boost/variant.hpp>

namespace storage
{

class PostgreSQL : public Backend<PostgreSQL>
{
    struct ConnectionManager{
        struct Connection
        {
            bool in_use=false;
            PGconn* pgconn=0;
            Connection();
            ~Connection();
        };
        string connection_string;
        vector<Connection> vecConn;
        std::mutex mtx;

        PGconn* try_get();
        PGconn* get();
        void release(PGconn* conn);
    };
    mutable ConnectionManager connection_;
public:
    PostgreSQL();

    void connection(string connection_string, short count=4);
    void close();

    string exec_sql(const string& sql) const;

    template<class T>
    string exec_sql(const string& sql, T* obj) const
    {
    #if DEBUG
        clog << __PRETTY_FUNCTION__ << sql << endl;
    #endif
        PGconn* conn = connection_.get();
        PGresult* res = PQexec(conn, sql.c_str());
        bool ok = verifyResult(res);
        connection_.release(conn);

        if(ok && PQntuples(res)>0 && obj!=NULL){
            int coll=0;
            setValues sv{res, 0, coll, PQnfields(res)};
            reflector::visit_each(*obj, sv);
         }

        string rowsAffected = PQcmdTuples(res);
        PQclear(res);

        if(!ok)
            throw_with_trace( runtime_error("PostgreSQL::exec_sql "+string(PQerrorMessage(conn))+"\n\tSQL: "+sql) );

        return rowsAffected;
    }

    template<class TypeRet>
    TypeRet exec_sql(const string& sql)
    {
        PGconn* conn = connection_.get();
        typedef typename TypeRet:: value_type TypeBean;
        TypeRet ret;
        PGresult* res = PQexec(conn, sql.c_str());
        bool ok = verifyResult(res);
        connection_.release(conn);
        if(ok){
            int rows = PQntuples(res);
            for(int l=0; l<rows; l++) {
                TypeBean obj;
                int coll=0;
                reflector::visit_each(obj, setValues(res, l, coll, PQnfields(res)));

                ret.emplace_back(std::move(obj));
            }
            PQclear(res);
        }
        if(!ok)
            throw_with_trace( runtime_error("PostgreSQL::exec_sql "+string(PQerrorMessage(conn))+"\n\tSQL: "+sql) );
        return ret;
    }

    void exec_sql(const string& sql, std::function<void(PGresult*, int, bool&)> callback);

    template<class TypeBean>
    void exec_sql(const string& sql, std::function<void(TypeBean&)> callback, bool subSet=false)
    {
        exec_sql(sql, [&](PGresult* res, int rows, bool& ok){
            TypeBean obj;
            for(int l=0; l<rows; l++) {
                int coll=0;
                reflector::visit_each(obj, setValues(res, l, coll, PQnfields(res), subSet));
                callback(obj);
            }
        });
    }

    template<class T>
    string getSqlInsert(T& bean) const
    {
        const string& pks = getListPK<T>(bean);
        return Backend<PostgreSQL>::getSqlInsertBase<T>(bean) + (pks.size()?" RETURNING "+pks:"");
    }
private:
    struct setValues
    {
        PGresult* res;
        const int &row;
        int& coll;
        int nColl;
        vector<int> entityesJoin;
        Reference* ref;
        bool subset;

        struct setSubField{
            Reference* ref;
            setValues* sv;

            template<class FieldData, class Annotations>
            auto operator()(FieldData f, Annotations a, int lenght) noexcept -> std::enable_if_t<is_simple_or_datatime_type<typename FieldData::type>::value>
            {
                if(ref->field == f.name())
                    sv->putValue(f.get(), lenght);
            }
            template<class FieldData, class Annotations>
            auto operator()(FieldData f, Annotations a, int lenght) noexcept -> std::enable_if_t<!is_simple_or_datatime_type<typename FieldData::type>::value>
            {}
        };

        setValues(PGresult* res, const int &row, int& coll, int nColl, bool subset=false) :
            res(res),
            row(row),
            coll(coll),
            nColl(nColl),
            subset(subset) {
        }


        template <class T>
        auto putValue(T& val, int& lenght) noexcept -> std::enable_if_t<is_simple_or_datatime_type<T>::value>
        {
            char* str = PQgetvalue(res, row, coll++);
            if(str==nullptr || strlen(str)==0)
                val = T{};
            else{
                stringstream ss;
                ss.imbue(delimit_endl);
                ss << str;
                ss >> val;
            }
        }

        template <class T>
        auto putValue(T& val, int& lenght) noexcept -> std::enable_if_t<!is_simple_or_datatime_type<T>::value>
        {
            if( subset || (nColl <= lenght) )
                reflector::visit_each(val, setSubField{ref, this});
            else
                reflector::visit_each(val, *this);
        }

        template<class FieldData, class Annotations>
        void operator()(FieldData f, Annotations a, int lenght)
        {
           const char* nome = f.name();
            ref = a.get_field(nome);
            if(coll < nColl){
                auto& val = f.get();
                putValue(val, lenght);
            }
        }
    };

    bool verifyResult(PGresult* res) const;
};

bool verifyResult(PGresult* res, PGconn *conn, const string &sql);

}
#endif // POSTGRESQL_H
