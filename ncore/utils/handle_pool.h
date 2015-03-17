#ifndef NCORE_UTILS_HANDLE_POOL_H_
#define NCORE_UTILS_HANDLE_POOL_H_

#include <ncore/ncore.h>

namespace ncore
{


template<typename Handle>
class HandlePool
{
public:
    HandlePool()
    {}

    Handle::Type Get()
    {
        Handle::Type handle = Handle::kInvalidHandle;
        if (pool_.size()) {
            handle = pool_.top();
            pool_.pop();
        }
        return handle;
    }

    void Put(Handle::Type handle)
    {
        if (handle != Handle::kInvalidHandle)
            pool_.push(handle);
    }

private:
    std::stack<HandleType> pool_;
};


}

#endif