#ifndef NCORE_SYS_NAMED_EVENT_H
#define NCORE_SYS_NAMED_EVENT_H

#include <ncore/ncore.h>
#include <ncore/base/object.h>
#include "waitable.h"

namespace ncore
{

struct NamedEventArgs
{
    const char * name;
    bool open_exists;
    bool manual_reset;
    bool signalled;
    bool none_privilege;

    NamedEventArgs();
};


class NamedEvent : public NonCopyableObject,
                   public Waitable
{
public:
    NamedEvent();
    ~NamedEvent();
    bool init(bool manual_reset, bool initial_state);
    bool init(const NamedEventArgs & args);
    void fini();
    bool Set();
    bool Reset();
    bool Wait(uint32_t timeout);
    bool WaitEx(uint32_t timeout, bool alertable);
    bool IsPremier();

    void * WaitableHandle() const;
        
private:
    HANDLE handle_;
};


}

#endif