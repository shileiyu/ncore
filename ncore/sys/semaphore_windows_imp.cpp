#include "wait.h"
#include "semaphore.h"

namespace ncore
{


Semaphore::Semaphore()
    : handle_(0)
{
}

Semaphore::~Semaphore()
{
    fini();
}

bool Semaphore::init(uint32_t initial, uint32_t max)
{
    if(handle_)
        return true;

    handle_ = CreateSemaphore(0, initial, max, 0);
    return handle_ != 0;
}

void Semaphore::fini()
{
    if(handle_)
    {
        ::CloseHandle(handle_);
        handle_ = 0;
    }
}

//'Increase' in Dutch
bool Semaphore::Verhogen(uint32_t release, uint32_t & previous)
{
    if(!handle_)
        return false;
    LPLONG prev = reinterpret_cast<LPLONG>(&previous);
    return ::ReleaseSemaphore(handle_, release, prev) != FALSE;
}

//alias for Verhogen
bool Semaphore::Increase(uint32_t release)
{
    uint32_t dontcare;
    return Verhogen(release, dontcare);
}

bool Semaphore::Wait(uint32_t timeout)
{
    return WaitEx(timeout, false);
}

bool Semaphore::WaitEx(uint32_t timeout, bool alertable)
{
    return Wait::WaitOne(*this, timeout, alertable);
}

void * Semaphore::WaitableHandle() const
{
    return handle_;
}

}