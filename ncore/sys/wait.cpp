#include "wait_exception.h"
#include "wait.h"

namespace ncore
{


bool Wait::WaitOne(Waitable & object)
{
    return WaitOne(object, (uint32_t)(-1));
}

bool Wait::WaitOne(Waitable & object, uint32_t time_out)
{
    return WaitOne(object, time_out, false);
}

bool Wait::WaitOne(Waitable & object, uint32_t time_out, bool alertable)
{
    DWORD result = 0;
    HANDLE handle = object.WaitableHandle();
    result = ::WaitForSingleObjectEx(handle, time_out, alertable);
    switch(result)
    {
    case WAIT_OBJECT_0:
    case WAIT_IO_COMPLETION:
        return true;
    case WAIT_TIMEOUT:
        return false;
    case WAIT_FAILED:
        throw WaitExceptionFailed();
        break;
    case WAIT_ABANDONED:
        throw WaitExceptionAbandon();
        break;
    default:
        abort();
        return false;
    }
    return false;
}


}