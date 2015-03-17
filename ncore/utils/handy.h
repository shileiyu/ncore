#ifndef NCORE_UTILS_HANDY_H_
#define NCORE_UTILS_HANDY_H_

#include <ncore/ncore.h>

namespace ncore
{

template<typename T> inline
size_t countof(const T & arr)
{
    return sizeof(arr) / sizeof(arr[0]);
}


template<typename S> inline
const typename S::value_type * r(const S & s)
{
    return s.data();
}

}

#endif