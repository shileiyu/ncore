#ifndef NCORE_SYS_MUTEX_H_
#define NCORE_SYS_MUTEX_H_

#include <ncore/ncore.h>
#include <ncore/base/object.h>
#include "waitable.h"

namespace ncore
{

/*进程内互斥体*/
class CriticalSection : public NonCopyableObject
{
public:
    CriticalSection();
    ~CriticalSection();
            
    bool init();
    bool init(int spin_count);
    void fini();

    bool Try();
    void Enter();
    void Leave();	
private:
#if defined NCORE_WINDOWS
    CRITICAL_SECTION cs_;
#endif
    bool initialize_;
};

struct MutexArgs
{
    const char * name;
    bool owned;
    bool none_privilege;
};

class Mutex
    : public Waitable
      
{
public:
    Mutex();
    ~Mutex();

    bool init(bool owned);

    bool init(const MutexArgs & args);

    void fini();

    bool Release();

    bool Wait(uint32_t time);

    bool WaitEx(uint32_t time, bool aletable);

    void * WaitableHandle() const;
private:
    HANDLE handle_;
};

/**/
class ScopedCriticalSection
{
public:
    ScopedCriticalSection(CriticalSection * cs)
        : cs_(0)
    {
        if(cs)
        {
            cs_ = cs;
            cs_->Enter();
        }
    }

    ~ScopedCriticalSection()
    {
        if(cs_)
        {
            cs_->Leave();
            cs_ = 0;
        }
    }
private:
    CriticalSection * cs_;
};


}

#endif