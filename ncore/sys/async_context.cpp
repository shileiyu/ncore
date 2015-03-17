#include "named_event.h"
#include "async_context.h"

namespace ncore
{


AsyncContext::AsyncContext()
    : user_token_(0)
{
    overlapped_.Internal = 0;
    overlapped_.InternalHigh = 0;
    overlapped_.Offset = 0;
    overlapped_.OffsetHigh = 0;
    overlapped_.hEvent = 0;
}

AsyncContext::AsyncContext(const NamedEvent & e)
    : user_token_(0)
{
    overlapped_.Internal = 0;
    overlapped_.InternalHigh = 0;
    overlapped_.Offset = 0;
    overlapped_.OffsetHigh = 0;
    overlapped_.hEvent = e.WaitableHandle();
}

void * AsyncContext::user_token() const
{
    return user_token_;
}

void AsyncContext::set_user_token(void * token)
{
    user_token_ = token;
}

void AsyncContext::SuppressIOCP()
{
    DWORD value = reinterpret_cast<DWORD>(overlapped_.hEvent);
    if(value)
    {
        value |= 0x1;
        overlapped_.hEvent = reinterpret_cast<HANDLE>(value);
    }
}

}
