#ifndef SERVICE_H
#define SERVICE_H
#include <map>

#include "function.h"

class Service
{
public:
#define REGISTER_FUNCTION(ser, id, f) Service::registerFunction(ser, id, f)
    virtual ~Service() = default;

    virtual int id() = 0;

    static void registerFunction(int ser, int id, FunctionBase *f)
    {
        registry[ser].registerFunction(id, f);
    }

    static  std::map<int, FunctionRegistry> registry;


};
std::map<int, FunctionRegistry> Service::registry;

#endif // SERVICE_H