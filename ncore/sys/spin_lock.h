#ifndef NCORE_SYS_SPIN_LOCK_H_
#define NCORE_SYS_SPIN_LOCK_H_

#include <ncore/base/atomic.h>

namespace ncore
{

class SpinLock
{
public:
    SpinLock();
    ~SpinLock();

    bool TryAcquire();
    void Acquire();
    void Release();
private:
    Atomic locker_;
};


}

#endif