
#ifndef ASYNC_NAMED_PIPE_RESULT_H_
#define ASYNC_NAMED_PIPE_RESULT_H_

#include <ncore/ncore.h>
#include <ncore/utils/async_result_delegate.h>
#include <ncore/utils/async_result_adapter.h>
#include "async_context.h"

namespace ncore
{

namespace AsyncNamedPipeOp
{
enum Value
{
    kAsyncPipeUnknow,
    kAsyncPipeAccept,
    kAsyncPipeRead,
    kAsyncPipeWrite
};
}

class NamedPipeAsyncContext;

typedef AsyncResultDelegate<NamedPipeAsyncContext> NamedPipeAsyncResultDelegate;
typedef AsyncResultHandler<NamedPipeAsyncContext>  NamedPipeAsyncResultHandler;

class NamedPipeAsyncContext : public AsyncContext
{
public:
    NamedPipeAsyncContext();
    NamedPipeAsyncContext(NamedEvent & e);

    void SetBuffer(const void * buffer, size_t count);
    void SetBuffer(void * buffer, size_t count);

    void set_completion_delegate(NamedPipeAsyncResultHandler * handler);

    void * data() const;
    size_t count() const;
    uint32_t error() const;
    uint32_t transfered() const;
    AsyncNamedPipeOp::Value last_op() const;

private:
    void OnCompleted(uint32_t error, uint32_t transfered);

private:
    void * data_;
    size_t count_;
    uint32_t error_;
    uint32_t transfered_;
    AsyncNamedPipeOp::Value last_op_;
    NamedPipeAsyncResultDelegate completion_delegate_;

    friend class NamedPipe;
    friend class NamedPipeServer;
    friend class NamedPipeClient;
    friend class NamedPipeRoutines;
};

template <typename Adaptee>
using NamedPipeAsyncResultAdapter = 
AsyncResultAdapter<Adaptee, NamedPipeAsyncContext>;

}

#endif
