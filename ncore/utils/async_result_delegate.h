#ifndef NCORE_UTILS_EVENT_DELEGATE_H_
#define NCORE_UTILS_EVENT_DELEGATE_H_

#include "async_result_handler.h"

namespace ncore
{

template<typename Context>
class AsyncResultDelegate
{
public:
    typedef AsyncResultHandler<Context> Handler;

    AsyncResultDelegate()
        : handler_(0)
    {
    }

    AsyncResultDelegate & operator=(Handler * handler)
    {
        handler_ = handler;
        return *this;
    }

    void operator()(Context & ctx)
    {
        if (handler_)
            handler_->OnEvent(ctx);
    }

    bool operator!=(void * ptr)
    {
        return handler_ != ptr;
    }

    bool operator==(void * ptr)
    {
        return handler_ == ptr;
    }

private:
    Handler * handler_;
};


}

#endif