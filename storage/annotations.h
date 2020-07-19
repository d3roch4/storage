#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H
#include <string>
#include <functional>

struct PrimaryKey{};

struct NotNull{};

struct IgnoreStorage{};

struct Indexed
{
    std::string using_;
    Indexed(std::string using_ =  {}) : using_(using_){}
};

struct Type
{
    std::string name;
    Type(std::string name): name(name) {}
};

struct Select
{
    std::string sql;
    Select(std::string sql): sql(sql) {}
};

struct Insert
{
    std::function<std::string(const std::string&)> converter;
    Insert(std::function<std::string(const std::string&)> converter)
        : converter(converter){
    }
};

#endif // ANNOTATIONS_H
