#ifndef NCORE_UTILS_EVENT_HANDLER_H_
#define NCORE_UTILS_EVENT_HANDLER_H_

#include <ncore/ncore.h>

namespace ncore
{

template<typename Context>
class AsyncResultHandler
{
public:
    virtual void OnEvent(Context & ctx) = 0;
};

}

#endif