#ifndef NCORE_SYS_WAITABLE_H_
#define NCORE_SYS_WAITABLE_H_

#include <ncore/ncore.h>

namespace ncore
{

class Waitable
{
    friend class Wait; 
private:
    virtual void * WaitableHandle() const = 0;
};


}

#endif