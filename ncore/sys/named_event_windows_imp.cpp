#include <ncore/encoding/utf8.h>
#include "wait.h"
#include "named_event.h"

namespace ncore
{

NamedEventArgs::NamedEventArgs()
{
    name = nullptr;
    open_exists = false;
    manual_reset = false;
    signalled = false;
    none_privilege = false;
}

//NamedEvent
NamedEvent::NamedEvent()
    : handle_(0)
{
}

NamedEvent::~NamedEvent()
{
    fini();
}

bool NamedEvent::init(bool manual_reset, bool initial_state)
{
    NamedEventArgs args;
    args.manual_reset = manual_reset;
    args.signalled = initial_state;
    return init(args);
}

bool NamedEvent::init(const NamedEventArgs & args)
{
    if (handle_)
        return true;

    wchar_t name16[kMaxPath16] = { 0 };
    if (args.name)
    UTF8::Decode(args.name, -1, name16, 260);

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;

    if (args.open_exists)
    {
        DWORD access = EVENT_MODIFY_STATE | SYNCHRONIZE;
        handle_ = OpenEvent(access, FALSE, args.name ? name16 : NULL);
    }
    else
    {
        handle_ = CreateEvent(
            args.none_privilege ? &sa : NULL,
            args.manual_reset, args.signalled,
            args.name ? name16 : NULL
        );
    }
    return handle_ != NULL;
}


void NamedEvent::fini()
{
    if(handle_)
    {
        CloseHandle(handle_);
        handle_ = 0;
    }
}

bool NamedEvent::Set()
{
    if(handle_ == NULL)
        return false;

    if (!SetEvent(handle_))
        return false;

    return true;
}

bool NamedEvent::Reset()
{
    if(handle_ == NULL)
        return false;

    if (!ResetEvent(handle_))
        return false;
         
    return true;
}

bool NamedEvent::Wait(uint32_t timeout)
{
    return WaitEx(timeout, false);
}

bool NamedEvent::WaitEx(uint32_t timeout, bool alertable)
{
    return Wait::WaitOne(*this, timeout, alertable);
}

void * NamedEvent::WaitableHandle() const
{
    return handle_;
}

bool NamedEvent::IsPremier()
{
    return ERROR_ALREADY_EXISTS != GetLastError();
}

}

