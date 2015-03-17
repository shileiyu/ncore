#ifndef NCORE_SYS_IO_PORTAL_H_
#define NCORE_SYS_IO_PORTAL_H_

#include <ncore/ncore.h>

namespace ncore
{

class AsyncContext;

class IOPortal
{
private:
    virtual void * GetPlatformHandle() = 0;
    virtual void OnCompleted(AsyncContext & args,
                             uint32_t error,
                             uint32_t transfered) = 0;

    friend class Proactor;
};

}

#endif