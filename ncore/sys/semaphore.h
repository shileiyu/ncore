#ifndef NSSAHRE_SYS_SEMAPHORE_H_
#define NSSAHRE_SYS_SEMAPHORE_H_

#include <ncore/ncore.h>
#include "waitable.h"

namespace ncore
{

class Semaphore : public Waitable
{
public:
    Semaphore();
    ~Semaphore();

    bool init(uint32_t initial, uint32_t max);
    void fini();

    //'Increase' in Dutch
    bool Verhogen(uint32_t release, uint32_t & previous);
    //alias for Verhogen
    bool Increase(uint32_t release);

    bool Wait(uint32_t timeout);
    bool WaitEx(uint32_t timeout, bool alertable);
    void * WaitableHandle() const;
private:
    HANDLE handle_;
};

}

#endif