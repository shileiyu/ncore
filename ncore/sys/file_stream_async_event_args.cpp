#include "file_stream_async_event_args.h"

namespace ncore
{


FileStreamAsyncContext::FileStreamAsyncContext()
    : data_(0), count_(0), error_(0), transfered_(0),
      completion_delegate_(), lock_mode_(), lock_size_(0),
      last_op_(AsyncFileStreamOp::kAsyncUnknow)
{
}

FileStreamAsyncContext::FileStreamAsyncContext(NamedEvent & e)
    : AsyncContext(e), data_(0), count_(0), error_(0), 
      transfered_(0), completion_delegate_(), lock_mode_(),
      lock_size_(0), last_op_(AsyncFileStreamOp::kAsyncUnknow)
{
}

void FileStreamAsyncContext::SetBuffer(void * buffer, size_t count)
{
    data_ = buffer;
    count_ = count;
}

void FileStreamAsyncContext::SetBuffer(const void * buffer, size_t count)
{
    data_ = const_cast<void *>(buffer);
    count_ = count;
}

void FileStreamAsyncContext::set_offset(uint32_t lo, uint32_t hi)
{
    overlapped_.Offset = lo;
    overlapped_.OffsetHigh = hi;
}

void FileStreamAsyncContext::set_offset(uint64_t offset)
{
    *(uint64_t*)(&overlapped_.Offset) = offset;
}

void FileStreamAsyncContext::set_lock_mode(FileLockMode mode)
{
    lock_mode_ = mode;
}

void FileStreamAsyncContext::set_lock_size(uint64_t size)
{
    lock_size_ = size;
}

void FileStreamAsyncContext::set_completion_delegate(
    FileStreamAsyncResultHandler * handler
) {
    completion_delegate_ = handler;
}

uint64_t FileStreamAsyncContext::offset() const
{
    return reinterpret_cast<uint64_t>(overlapped_.Pointer);
}

void * FileStreamAsyncContext::data() const
{
    return data_;
}

size_t FileStreamAsyncContext::count() const
{
    return count_;
}

uint32_t FileStreamAsyncContext::error() const
{
    return error_;
}

uint32_t FileStreamAsyncContext::transfered() const
{
    return transfered_;
}

FileLockMode FileStreamAsyncContext::lock_mode() const
{
    return lock_mode_;
}

uint64_t FileStreamAsyncContext::lock_size() const
{
    return lock_size_;
}

AsyncFileStreamOp::Value FileStreamAsyncContext::last_op() const
{
    return last_op_;
}

void FileStreamAsyncContext::OnCompleted(uint32_t error, uint32_t transfered)
{
    error_ = error;
    transfered_ = transfered;
    completion_delegate_(*this);
}


}