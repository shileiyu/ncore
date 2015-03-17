#ifndef NCORE_SYS_ASYNC_DIRECTORY_RESULT_H_
#define NCORE_SYS_ASYNC_DIRECTORY_RESULT_H_

#include <ncore/ncore.h>
#include <ncore/utils/async_result_delegate.h>
#include <ncore/utils/async_result_adapter.h>
#include "async_context.h"
#include "file_define.h"

namespace ncore
{


class Event;
class DirectoryAsyncContext;

typedef AsyncResultDelegate<DirectoryAsyncContext> DirectoryAsyncResultDelegate;
typedef AsyncResultHandler<DirectoryAsyncContext>  DirectoryAsyncResultHandler;

class DirectoryAsyncContext : public AsyncContext
{
public:
    DirectoryAsyncContext();
    DirectoryAsyncContext(NamedEvent & e);

    void SetBuffer(void * data, size_t count);

    void set_option(FileChangesNotify option);
    void set_completion_delegate(DirectoryAsyncResultHandler * handler);

    void * data() const;
    size_t count() const;
    uint32_t error() const;
    uint32_t transfered() const;
    FileChangesNotify option() const;

private:
    void OnCompleted(uint32_t error, uint32_t transfered);
private:
    void * data_;
    size_t count_;
    uint32_t error_;
    uint32_t transfered_;
    FileChangesNotify option_;
    DirectoryAsyncResultDelegate completion_delegate_;

    friend class Directory;
    friend class DirectoryRoutines;
};


}

#endif