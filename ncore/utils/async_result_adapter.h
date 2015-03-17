#ifndef NCORE_UTILS_EVENT_ADAPTER_H_
#define NCORE_UTILS_EVENT_ADAPTER_H_

#include "async_result_handler.h"

namespace ncore
{


template<typename T, typename Context>
class AsyncResultAdapter : public AsyncResultHandler<Context>
{
public:
    typedef void (T::*F)(Context & ctx);
public:
    AsyncResultAdapter()
        : obj_(0), func_(0)
    {
    }

    void Register(T * obj, F func)
    {
        obj_ = obj;
        func_ = func;
    }

    void OnEvent(Context & ctx)
    {
        assert(obj_);
        assert(func_);

        if(obj_ && func_)
            (obj_->*func_)(ctx);
    }

private:
    T * obj_;
    F func_;
};

}

#endif