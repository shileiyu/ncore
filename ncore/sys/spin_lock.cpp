#include "spin_lock.h"

namespace ncore
{

SpinLock::SpinLock()
{
}

SpinLock::~SpinLock()
{
}

bool SpinLock::TryAcquire()
{
    return !locker_.CompareExchange(1, 0);
}

void SpinLock::Acquire()
{
    while(locker_.CompareExchange(1, 0));
}

void SpinLock::Release()
{
    locker_ = 0;
}

}