#include "io_portal.h"
#include "thread.h"
#include "proactor.h"

namespace ncore
{

/*
前摄器
*/
Proactor::Proactor() : comp_port_(0)
{
}

Proactor::~Proactor()
{
    fini();
}

bool Proactor::init()
{
    if(comp_port_)
        return true;

    comp_port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

    return comp_port_ ? true : false;
}

void Proactor::fini()
{
    if(comp_port_)
    {
        CloseHandle(comp_port_);
        comp_port_ = 0;
    }
    return;
}

void Proactor::Run(int ms)
{
    assert(comp_port_ != 0);

    if(comp_port_ == 0)
        return;

    BOOL status = false;
    DWORD transfered = 0;
    ULONG_PTR comp_key = 0;
    LPOVERLAPPED overlapped = 0;

    status = GetQueuedCompletionStatus(comp_port_,
                                       &transfered,
                                       &comp_key,
                                       &overlapped,
                                       ms);

    if(overlapped)
    {
        auto & args = *reinterpret_cast<AsyncContext*>(overlapped);
        auto portal = reinterpret_cast<IOPortal*>(comp_key);
        auto error = status ? 0 : GetLastError();

        portal->OnCompleted(args, error, transfered);
    }
    else
    {
        uint32_t err = GetLastError();
        if(err == WAIT_TIMEOUT)
            Thread::Sleep(0, true);
    }
}

bool Proactor::Associate(IOPortal & portal)
{
    if(comp_port_ == 0)
        return false;
    
    HANDLE handle = portal.GetPlatformHandle();
    ULONG_PTR comp_key = reinterpret_cast<ULONG_PTR>(&portal);

    if(!CreateIoCompletionPort(handle, comp_port_, comp_key, 0))
        return false;

    return true;
}
 
}

