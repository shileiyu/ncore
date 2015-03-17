#include <ncore/encoding/utf8.h>
#include <ncore/utils/karma.h>
#include "named_event.h"
#include "proactor.h"
#include "directory_async_event_args.h"
#include "directory.h"

namespace ncore
{


class DirectoryRoutines
{
private:
    friend class Directory;

    static DirectoryAsyncContext & FromeOverlapped(LPOVERLAPPED lpOverlapped)
    {
        return *reinterpret_cast<DirectoryAsyncContext*>(lpOverlapped);
    }

    static void __stdcall OnCompleted(DWORD dwErrorCode,
                                      DWORD dwBytesTransfered,
                                      LPOVERLAPPED lpOverlapped)
    {
        DirectoryAsyncContext & dae = FromeOverlapped(lpOverlapped);
        dae.OnCompleted(dwErrorCode, dwBytesTransfered);
    }
};

Directory::Directory()
    : handle_(INVALID_HANDLE_VALUE), io_handler_(0)
{
}

Directory::~Directory()
{
    fini();
}

bool Directory::init(const char * path)
{

    if(handle_ != INVALID_HANDLE_VALUE)
        return true;

    if(!path)
        return false;

    char name[kMaxPath8];
    name[0] = 0;
    Karma::Copy(name, path);

    size_t name_length = strlen(name);
    if(!name_length)
        return false;
    
    if(name[name_length] == '\\' ||
       name[name_length] == '/')
    {
        name[name_length] = 0;
    }

    wchar_t name16[kMaxPath16];
    if(!UTF8::Decode(name, -1, name16, kMaxPath16))
        return false;

    DWORD flag = FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS;
    DWORD share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
    handle_ = CreateFile(name16, FILE_LIST_DIRECTORY, share, 0, 
                         OPEN_EXISTING, flag, 0);

    if(handle_ != INVALID_HANDLE_VALUE)
        return true;

    return false;
}

void Directory::fini()
{
    if(handle_ != INVALID_HANDLE_VALUE)
    {
        auto tmphandle = handle_;
        handle_ = INVALID_HANDLE_VALUE;
        ::CloseHandle(tmphandle);
    }
}

bool Directory::ReadChanges(void * data, uint32_t size_to_read, 
                            uint32_t & transfered)
{
    return ReadChanges(data, size_to_read, -1, transfered);
}

bool Directory::ReadChanges(void * data, uint32_t size_to_read, 
                            uint32_t timeout, uint32_t & transfered)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(data == 0)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    DirectoryAsyncContext args(complete_event);
    args.SetBuffer(data, size_to_read);
    
    if(!ReadChangesAsync(args))
        return false;

    if(!WaitDirectoryAsyncEvent(args))
        return false;

    transfered = args.transfered();
    return true;
}

bool Directory::ReadChangesAsync(DirectoryAsyncContext & args)
{
    auto option = args.option();
    BOOL watch_subtree = !!(option & FileChangesNotify::kWatchSubtree);
    DWORD filter = option & ~FileChangesNotify::kWatchSubtree;

    auto iocr = DirectoryRoutines::OnCompleted;
    if(args.completion_delegate_ == 0 || io_handler_ != 0)
        iocr = 0;

    if(!ReadDirectoryChangesW(
        handle_, args.data(), args.count(), watch_subtree,
        filter, 0, &args.overlapped_, iocr
    )) {
        return false;
    }

    return true;
}

bool Directory::IsValid()
{
    return handle_ != INVALID_HANDLE_VALUE;
}

bool Directory::Cancel()
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    return ::CancelIo(handle_) != FALSE;
}

bool Directory::Associate(Proactor & io)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(io.Associate(*this))
    {
        io_handler_ = &io;
        return true;
    }
    return false;
}


}