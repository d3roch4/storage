#ifndef BACKEND_H
#define BACKEND_H

#include <memory>
#include "orm.h"

using namespace std;

class Backend
{
public:
    Backend();
    string getSqlInsert(const string& table, const vector<Column>& columns);
    string getSqlUpdate(const string& table, const vector<Column>& columns);
};

#endif // BACKEND_H
