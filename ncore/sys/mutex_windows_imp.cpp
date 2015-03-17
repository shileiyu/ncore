#include <ncore/encoding/utf8.h>
#include "wait.h"
#include "mutex.h"

namespace ncore
{


CriticalSection::CriticalSection()
    :initialize_(false)
{
    memset(&cs_, 0 ,sizeof(cs_));
}

CriticalSection::~CriticalSection()
{
    fini();
}

bool CriticalSection::init()
{
    assert(initialize_ == false);

    bool ret = InitializeCriticalSectionAndSpinCount(&cs_, 4000) != FALSE;
    initialize_ = true;
    return ret;
}

bool CriticalSection::init(int spin_count)
{
    assert(initialize_ == false);

    bool ret = InitializeCriticalSectionAndSpinCount(&cs_, spin_count) != FALSE;
    initialize_ = true;
    return ret;
}

void CriticalSection::fini()
{
    if(initialize_)
    {
        DeleteCriticalSection(&cs_);
        initialize_ = false;
    }
}

bool CriticalSection::Try()
{
    if(initialize_)
    {
        return TryEnterCriticalSection(&cs_) != FALSE;
    }

    return false;
}

void CriticalSection::Enter()
{
    if(initialize_)
    {
        EnterCriticalSection(&cs_);
    }
}

void CriticalSection::Leave()
{
    if(initialize_)
    {
        LeaveCriticalSection(&cs_);
    }
}

/*
Mutex
*/
Mutex::Mutex()
    : handle_(0)
{
}

Mutex::~Mutex()
{
    fini();
}

bool Mutex::init(bool owned)
{
    if(handle_)
        return true;

    MutexArgs args;
    args.name = nullptr;
    args.owned = owned;
    args.none_privilege = false;
    return init(args);
}

bool Mutex::init(const MutexArgs & args)
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

    handle_ = CreateMutex(
        args.none_privilege ? &sa : NULL,
        args.owned, args.name ? name16 : NULL
    );

    return handle_ != NULL;
}

void Mutex::fini()
{
    if(handle_)
    {
        CloseHandle(handle_);
        handle_ = 0;
    }
}

bool Mutex::Release()
{
    if(handle_)
        return ::ReleaseMutex(handle_) != FALSE;
    return false;
}

bool Mutex::Wait(uint32_t time)
{
    return Wait::WaitOne(*this, time, false);
}

bool Mutex::WaitEx(uint32_t time, bool aletable)
{
    return Wait::WaitOne(*this, time, aletable);
}

void * Mutex::WaitableHandle() const
{
    return handle_;
}


}