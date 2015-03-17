#include "directory_async_event_args.h"

namespace ncore
{


DirectoryAsyncContext::DirectoryAsyncContext()
    : data_(0), count_(0), error_(0), transfered_(0),
      option_()
{
}

DirectoryAsyncContext::DirectoryAsyncContext(NamedEvent & e)
    : AsyncContext(e), data_(0), count_(0), error_(0), 
      transfered_(0), option_()
{
}

void DirectoryAsyncContext::SetBuffer(void * data, size_t count)
{
    data_ = data;
    count_ = count;
}

void DirectoryAsyncContext::set_option(FileChangesNotify option) 
{
    option_ = option;
}

void DirectoryAsyncContext::set_completion_delegate(
    DirectoryAsyncResultHandler * handler
) {
    completion_delegate_ = handler;
}

void * DirectoryAsyncContext::data() const
{
    return data_;
}

size_t DirectoryAsyncContext::count() const
{
    return count_;
}

uint32_t DirectoryAsyncContext::error() const
{
    return error_;
}

uint32_t DirectoryAsyncContext::transfered() const
{
    return transfered_;
}

FileChangesNotify DirectoryAsyncContext::option() const
{
    return option_;
}

void DirectoryAsyncContext::OnCompleted(uint32_t error, uint32_t transfered)
{
    error_ = error;
    transfered_ = transfered;
    completion_delegate_(*this);
}


}