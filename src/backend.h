#ifndef BACKEND_H
#define BACKEND_H

#include <memory>
#include "orm.h"

using namespace std;

class Backend
{
public:
    Backend();
    string getSqlInsert(const string& table, const vector<shared_ptr<iColumn> > &columns);
    string getSqlUpdate(const string& table, const vector<shared_ptr<iColumn> > &columns);
};

#endif // BACKEND_H
