#ifndef NCORE_UTILS_SINGLETON_H_
#define NCORE_UTILS_SINGLETON_H_

#include <ncore/ncore.h>

namespace ncore
{

template<typename T>
class Singleton
{
public:
    static T & Get()
    {
        static T t;
        return t;
    }
};

}

#endif