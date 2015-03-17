#ifndef NCORE_SYS_ASYNC_FILE_RESULT_H_
#define NCORE_SYS_ASYNC_FILE_RESULT_H_

#include <ncore/ncore.h>
#include <ncore/utils/async_result_delegate.h>
#include <ncore/utils/async_result_adapter.h>
#include "async_context.h"
#include "file_define.h"


namespace ncore
{


class FileStream;

/*文件异步操作状态基类*/
namespace AsyncFileStreamOp
{
enum Value
{
    kAsyncUnknow,
    kAsyncWrite,
    kAsyncRead,
    kAsyncLock,
    kAsyncUnlock,
};
}

class Event;
class FileStreamAsyncContext;

typedef AsyncResultDelegate<FileStreamAsyncContext> FileStreamAsyncResultDelegate;
typedef AsyncResultHandler<FileStreamAsyncContext>  FileStreamAsyncResultHandler;

class FileStreamAsyncContext : public AsyncContext
{
public:
    FileStreamAsyncContext();
    FileStreamAsyncContext(NamedEvent & e);
    
    void SetBuffer(void * buffer, size_t count);
    void SetBuffer(const void * buffer, size_t count);

    void set_offset(uint32_t lo, uint32_t hi);
    void set_offset(uint64_t offset);
    void set_lock_mode(FileLockMode mode);
    void set_lock_size(uint64_t size);
    void set_completion_delegate(FileStreamAsyncResultHandler * handler);

    void * data() const;
    size_t count() const;
    uint64_t offset() const;
    uint32_t error() const;
    uint32_t transfered() const;
    FileLockMode lock_mode() const;
    uint64_t lock_size() const;
    AsyncFileStreamOp::Value last_op() const;

private:
    void OnCompleted(uint32_t error, uint32_t transfered);
private:
    void * data_;
    size_t count_;
    uint32_t error_;
    uint32_t transfered_;
    FileStreamAsyncResultDelegate completion_delegate_;
    FileLockMode lock_mode_;
    uint64_t lock_size_;
    AsyncFileStreamOp::Value last_op_;

    friend class FileStreamRoutines;
    friend class FileStream;
};

template<typename Adaptee>
using FileStreamAsyncResultAdapter =
AsyncResultAdapter<Adaptee, FileStreamAsyncContext>;


}
#endif