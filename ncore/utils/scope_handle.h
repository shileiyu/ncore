#ifndef NCORE_UTILS_SCOPE_HANDLE_H_
#define NCORE_UTILS_SCOPE_HANDLE_H_

#include <ncore/ncore.h>

namespace ncore
{

template<typename Handle>
class ScopeHandle
{
public:
    typedef typename Handle::Type HandleType;

    ScopeHandle()
        : handle_(Handle::kInvalidHandle) 
    {}

    ScopeHandle(HandleType handle)
        : handle_(handle)
    {}

    ~ScopeHandle() 
    { 
        if(handle_ != Handle::kInvalidHandle)
        {
            Handle::Free(handle_);
            handle_ = Handle::kInvalidHandle;
        }
    }

    ScopeHandle & operator=(HandleType handle)
    {
        handle_ = handle;
    }

    HandleType * operator&()
    {
        return &handle_;
    }

    bool operator==(HandleType handle)
    {
        return handle_ == handle;
    }

    operator HandleType() const
    {
        return handle_;
    }

    bool IsValid() const
    {
        if(handle_ == Handle::kInvalidHandle)
            return false;
        return true;
    }
private:
    HandleType handle_;
};

#ifdef NCORE_WINDOWS
class Win32GeneralHandle
{
public:
    typedef HANDLE Type;
    static const Type kInvalidHandle;
    static bool Free(Type handle)
    {
        return CloseHandle(handle) != FALSE;
    }
};

const Win32GeneralHandle::Type Win32GeneralHandle::kInvalidHandle = 0;

#endif


}

#endif