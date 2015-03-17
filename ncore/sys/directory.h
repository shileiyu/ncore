#ifndef NCORE_SYS_DIRECTORY_H_
#define NCORE_SYS_DIRECTORY_H_

#include <ncore/ncore.h>
#include <ncore/base/object.h>
#include "io_portal.h"
#include "file_define.h"

namespace ncore
{

class Proactor;
class DirectoryAsyncContext;

class Directory : public NonCopyableObject,
                  public IOPortal
{
public:
    static const size_t kMaxBuffer = 0x40000;
    static const size_t kMinBuffer = 0x1000;
#ifdef NCORE_WINDOWS
    typedef HANDLE HandleType;
#endif
public:
    Directory();
    ~Directory();

    bool init(const char * path);

    void fini();

    bool ReadChanges(void * data, uint32_t size_to_read, uint32_t & transfered);

    bool ReadChanges(
        void * data, uint32_t size_to_read,
        uint32_t timeout, uint32_t & transfered
    );

    bool ReadChangesAsync(DirectoryAsyncContext & args);

    //是否有效
    bool IsValid();

    //取消异步IO
    bool Cancel();

    bool Associate(Proactor & io);
private:
    bool WaitDirectoryAsyncEvent(DirectoryAsyncContext & args);
private:
    HandleType handle_;
    Proactor * io_handler_;
};


}

#endif