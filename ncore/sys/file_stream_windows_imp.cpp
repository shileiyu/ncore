#include <ncore/encoding/utf8.h>
#include "named_event.h"
#include "proactor.h"
#include "path.h"
#include "file_stream_async_event_args.h"
#include "file_stream.h"

namespace ncore
{


class FileStreamRoutines
{
private:
    friend class FileStream;

    static FileStreamAsyncContext & FromOverlapped(LPOVERLAPPED overlapped)
    {
        return *reinterpret_cast<FileStreamAsyncContext *>(overlapped);
    }

    static void __stdcall OnCompleted(DWORD dwErrorCode, 
                                      DWORD BytesTransfered, 
                                      LPOVERLAPPED lpOverlapped)
    {
        if(lpOverlapped)
        {
            FileStreamAsyncContext & fsae = FromOverlapped(lpOverlapped);
            fsae.OnCompleted(dwErrorCode, BytesTransfered);
        }
    }

    static FILETIME DateTimeToFileTime(const DateTime & dt)
    {
        FILETIME ft;
        int64_t * tick = reinterpret_cast<int64_t *>(&ft);
        *tick = dt.tick() * 10;
        return ft;
    }

    static DateTime FileTimeToDateTime(const FILETIME & ft)
    {
        const int64_t * tick = reinterpret_cast<const int64_t *>(&ft);
        return DateTime(*tick / 10);
    }
};


void FileStream::InvokeIOCompleteRoution()
{
    ::SleepEx(0,TRUE);
}

FileStream::FileStream()
    : handle_(INVALID_HANDLE_VALUE), io_handler_(0)
{

}

FileStream::~FileStream()
{
    fini();
}

FileStream::FileStream(FileStream && obj)
    : handle_(INVALID_HANDLE_VALUE)
{
    std::swap(handle_, obj.handle_);
}

FileStream & FileStream::operator = (FileStream && obj)
{
    std::swap(handle_, obj.handle_);
    return *this;
}

bool FileStream::init(const char * filename, 
                      FileAccess access,
                      FileShare share,
                      FileMode mode,
                      FileAttribute attr,
                      FileOption option)
{

    if(handle_ != INVALID_HANDLE_VALUE)
        return true;
    
    char normalized[kMaxPath8];
    if(!Path::NormalizePath(filename, normalized, kMaxPath8))
        return false;

    if(mode != FileMode::kOpen)
    {
        //先创建目录
        auto path = Path::GetPathFromFullName(normalized);
        if(!path.empty())
            Path::CreateDirectoryRecursive(path.data());
    }

    wchar_t filename16[kMaxPath16];
    if(!UTF8::Decode(normalized, -1, filename16, kMaxPath16))
        return false;


    DWORD flag = attr | option | FILE_FLAG_OVERLAPPED;
    handle_ = CreateFile(filename16, access, share, 0, mode, flag, 0);

    if(handle_ != INVALID_HANDLE_VALUE)
        return true;

    return false;
}

void FileStream::fini()
{
    auto sh = handle_;
    if(sh != INVALID_HANDLE_VALUE)
    {
        handle_ = INVALID_HANDLE_VALUE;
        CloseHandle(sh);
    }
}

bool FileStream::Flush()
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    return FlushFileBuffers(handle_) != FALSE;
}

bool FileStream::SetFileSize(uint64_t size)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;
    LARGE_INTEGER pos = {0};
    LARGE_INTEGER newpos = {0};
    pos.QuadPart = size;
    if(!SetFilePointerEx(handle_, pos, &newpos, FILE_BEGIN))
        return false;

    return SetEndOfFile(handle_) != FALSE;
}

bool FileStream::Truncate()
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;
    return SetEndOfFile(handle_) != FALSE;
}

bool FileStream::Seek(int64_t & position, FilePosition file_position)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    LARGE_INTEGER pos = {0};
    LARGE_INTEGER new_pos = {0};
    pos.QuadPart = position;
    if(!SetFilePointerEx(handle_, pos, &new_pos, file_position))
        return false;

    position = new_pos.QuadPart;
    return true;
}

bool FileStream::SetFilePos(uint64_t position)
{
    return Seek(reinterpret_cast<int64_t&>(position), FilePosition::kBegin);
}

bool FileStream::Tell(uint64_t & pos) const
{
    pos = 0;
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;
 
    LARGE_INTEGER curr_pos = {0};
    if(SetFilePointerEx(handle_, curr_pos, &curr_pos, FilePosition::kCurrent))
    {
        pos = curr_pos.QuadPart;
        return true;
    }
    return false;
}

bool FileStream::GetFileSize(uint64_t & file_size) const
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    LARGE_INTEGER  size = {0};
    if(::GetFileSizeEx(handle_, &size) != 0)
    {
        file_size = size.QuadPart;
        return true;
    }
    return  false;
}

bool FileStream::Read(void * data, uint32_t size_to_read, 
                      uint32_t & transfered)
{
    uint64_t pos = 0;
    if(!Tell(pos))
        return false;
    if(!Read(data, size_to_read, pos, transfered))
        return false;
    int64_t new_pos = pos + transfered;
    return Seek(new_pos, FilePosition::kBegin);
}

bool FileStream::Read(void * data, uint32_t size_to_read, 
                      uint64_t offset, uint32_t & transfered)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(data == 0)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    FileStreamAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.SetBuffer(data, size_to_read);
    args.set_offset(offset);
    
    if(!ReadAsync(args))
    {
        //Asynchronous Disk I/O Appears as Synchronous
        //on Windows NT, Windows 2000, and Windows XP
        //see: http://support.microsoft.com/kb/156932
        if(GetLastError() != ERROR_HANDLE_EOF)
            return false;
        ::SetEvent(args.overlapped_.hEvent);
    }

    if(!WaitFileStreamAsyncEvent(args))
        return false;

    transfered = args.transfered();
    return true;
}

bool FileStream::ReadAsync(FileStreamAsyncContext & args)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(args.data() == 0)
        return false;

    args.last_op_ = AsyncFileStreamOp::kAsyncRead;

    BOOL comp_synch = FALSE;
    if(args.completion_delegate_ != 0 && io_handler_ == 0)
    {
        auto iocr = FileStreamRoutines::OnCompleted;
        comp_synch = ReadFileEx(handle_, args.data(), args.count(),
                                &args.overlapped_, iocr);
    }
    else
    {
        comp_synch = ReadFile(handle_, args.data(), args.count(), 0, 
                              &args.overlapped_);
    }

    if(!comp_synch)
    {
        DWORD last_err = GetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    return true;
}

bool FileStream::Write(const void * data, uint32_t size_to_write, 
                       uint32_t & transfered)
{
    uint64_t pos = 0;
    if(!Tell(pos))
        return false;
    if(!Write(data, size_to_write, pos, transfered))
        return false;

    int64_t new_pos = pos + transfered;
    return Seek(new_pos, FilePosition::kBegin);
}

bool FileStream::Write(const void * data, uint32_t size_to_write, 
                       uint64_t offset, uint32_t & transfered)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(data == 0)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    FileStreamAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.SetBuffer(data, size_to_write);
    args.set_offset(offset);
    
    if(!WriteAsync(args))
        return false;

    if(!WaitFileStreamAsyncEvent(args))
        return false;

    transfered = args.transfered();    
    return true;
}

bool FileStream::WriteAsync(FileStreamAsyncContext & args)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(args.data() == 0)
        return false;

    args.last_op_ = AsyncFileStreamOp::kAsyncWrite;

    BOOL comp_synch = FALSE;
    if(args.completion_delegate_ != 0 && io_handler_ == 0)
    {
        auto iocr = FileStreamRoutines::OnCompleted;
        comp_synch = WriteFileEx(handle_, args.data(), args.count(), 
                                 &args.overlapped_, iocr);
    }
    else
    {
        comp_synch = WriteFile(handle_, args.data(), args.count(), 0,
                               &args.overlapped_);
    }

    if(!comp_synch)
    {
        DWORD last_err = GetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    return true;
}

bool FileStream::LockFile(uint64_t offset, uint64_t size)
{
    FileLockMode mode;
    mode.Set(FileLockMode::kExclusiveLock);
    mode.Set(FileLockMode::kFailImmediately);

    return LockFile(offset, size, mode);
}

bool FileStream::LockFile(uint64_t offset, uint64_t size, 
                          FileLockMode lock_mode)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    FileStreamAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.set_offset(offset);
    args.set_lock_mode(lock_mode);
    args.set_lock_size(size);

    if(!LockFileAsync(args))
        return false;

    if(!WaitFileStreamAsyncEvent(args))
        return false;

    return true;
}

bool FileStream::LockFileAsync(FileStreamAsyncContext & args)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    uint32_t size_lo = static_cast<uint32_t>(args.lock_size());
    uint32_t size_hi = static_cast<uint32_t>(args.lock_size() >> 32);

    args.last_op_ = AsyncFileStreamOp::kAsyncLock;
    if(!LockFileEx(handle_, args.lock_mode(), 0, size_lo, size_hi,
                   &args.overlapped_))
    {
        uint32_t last_err = GetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    else
    {
        args.OnCompleted(0, 0);
    }

    return true;
}

bool FileStream::UnlockFile(uint64_t offset, uint64_t size)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    FileStreamAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.set_lock_size(size);
    args.set_offset(offset);

    if(!UnlockFileAsync(args))
        return false;

    if(!WaitFileStreamAsyncEvent(args))
        return false;

    return true;
}

bool FileStream::UnlockFileAsync(FileStreamAsyncContext & args)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    uint32_t size_lo = static_cast<uint32_t>(args.lock_size());
    uint32_t size_hi = static_cast<uint32_t>(args.lock_size() >> 32);

    args.last_op_ = AsyncFileStreamOp::kAsyncUnlock;
    if(!UnlockFileEx(handle_, 0, size_lo, size_hi, &args.overlapped_))
    {
        uint32_t last_err = GetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    else
    {
        args.OnCompleted(0, 0);
    }

    return true;
}

bool FileStream::SetCreationTime(const DateTime & dt)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;
    FILETIME ft = FileStreamRoutines::DateTimeToFileTime(dt);
    return ::SetFileTime(handle_, &ft, 0, 0) != FALSE;
}

bool FileStream::GetCreationTime(DateTime & dt)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    FILETIME ft = {0};
    if(!::GetFileTime(handle_, &ft, 0, 0))
        return false;
    dt = FileStreamRoutines::FileTimeToDateTime(ft);
    return dt.IsValid();
}

bool FileStream::SetLastAccessTime(const DateTime & dt)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    FILETIME ft = FileStreamRoutines::DateTimeToFileTime(dt);
    return ::SetFileTime(handle_, 0, &ft, 0) != FALSE;
}

bool FileStream::GetLastAccessTime(DateTime & dt)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    FILETIME ft = {0};
    if(!::GetFileTime(handle_, 0, &ft, 0))
        return false;
    dt = FileStreamRoutines::FileTimeToDateTime(ft);
    return dt.IsValid();
}

bool FileStream::SetLastWriteTime(const DateTime & dt)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    FILETIME ft = FileStreamRoutines::DateTimeToFileTime(dt);
    return ::SetFileTime(handle_, 0, 0, &ft) != FALSE;
}

bool FileStream::GetLastWriteTime(DateTime & dt)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    FILETIME ft = {0};
    if(!::GetFileTime(handle_, 0, 0, &ft))
        return false;
    dt = FileStreamRoutines::FileTimeToDateTime(ft);
    return dt.IsValid();
}

bool FileStream::IsValid()
{
    return handle_ != INVALID_HANDLE_VALUE;
}

bool FileStream::Cancel()
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    return ::CancelIo(handle_) != FALSE;
}

bool FileStream::Associate(Proactor & io)
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

void * FileStream::GetPlatformHandle()
{
    return handle_;
}

void FileStream::OnCompleted(AsyncContext & args,
                             uint32_t error, 
                             uint32_t transfered)
{
    auto & file_stream_args = static_cast<FileStreamAsyncContext&>(args);
    file_stream_args.OnCompleted(error, transfered);
}

bool FileStream::WaitFileStreamAsyncEvent(FileStreamAsyncContext & args)
{
    bool succeed = true;
    DWORD error = 0;
    DWORD transed = 0;

    if(!GetOverlappedResult(handle_, &args.overlapped_, &transed, TRUE))
    {
        error = GetLastError();
        if(error != ERROR_HANDLE_EOF)
            succeed = false;
    }
    OnCompleted(args, error, transed);
    return succeed;
}


}
