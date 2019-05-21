#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H
#include <string>

struct PrimaryKey{};

struct NotNull{};

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

#endif // ANNOTATIONS_H
