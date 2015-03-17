#include "named_pipe_async_event_args.h"

namespace ncore
{


NamedPipeAsyncContext::NamedPipeAsyncContext()
    : data_(0), count_(0), error_(0), transfered_(0),
      last_op_(AsyncNamedPipeOp::kAsyncPipeUnknow)
{

}
NamedPipeAsyncContext::NamedPipeAsyncContext(NamedEvent & e)
    : AsyncContext(e), count_(0), error_(0), transfered_(0),
      last_op_(AsyncNamedPipeOp::kAsyncPipeUnknow)
{
}

void NamedPipeAsyncContext::SetBuffer(const void * buffer, size_t count)
{
    data_ = const_cast<void *>(buffer);
    count_ = count;
}

void NamedPipeAsyncContext::SetBuffer(void * buffer, size_t count)
{
    data_ = buffer;
    count_ = count;
}

void NamedPipeAsyncContext::set_completion_delegate(
    NamedPipeAsyncResultHandler * handler
) {
    completion_delegate_ = handler;
}

void * NamedPipeAsyncContext::data() const
{
    return data_;
}

size_t NamedPipeAsyncContext::count() const
{
    return count_;
}

uint32_t NamedPipeAsyncContext::error() const
{
    return error_;
}

uint32_t NamedPipeAsyncContext::transfered() const
{
    return transfered_;
}

AsyncNamedPipeOp::Value NamedPipeAsyncContext::last_op() const
{
    return last_op_;
}

void NamedPipeAsyncContext::OnCompleted(uint32_t error, uint32_t transfered)
{
    error_ = error;
    transfered_ = transfered;
    completion_delegate_(*this);
}


}