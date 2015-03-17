#include <ncore/encoding/utf8.h>
#include "named_event.h"
#include "proactor.h"
#include "named_pipe_async_event_args.h"
#include "named_pipe.h"

namespace ncore
{


class NamedPipeRoutines
{
private:
    friend class NamedPipe;
    friend class NamedPipeServer;
    friend class NamedPipeClient;

    static NamedPipeAsyncContext & FromOverlapped(LPOVERLAPPED overlapped)
    {
        return *reinterpret_cast<NamedPipeAsyncContext*>(overlapped);
    }

    static void __stdcall OnCompleted(DWORD dwErrorCode,
                                      DWORD BytesTransfered,
                                      LPOVERLAPPED lpOverlapped)
    {
        if(lpOverlapped)
        {
            NamedPipeAsyncContext & nae = FromOverlapped(lpOverlapped);
            nae.OnCompleted(dwErrorCode, BytesTransfered);
        }
        return;
    }
};

void NamedPipe::InvokeIOCompleteRoution()
{
    ::SleepEx(0,TRUE);
}

NamedPipe::NamedPipe() 
    : handle_(INVALID_HANDLE_VALUE), io_handler_(0)
{   
}

NamedPipe::~NamedPipe()
{
}

bool NamedPipe::Read(void * buffer, uint32_t size_to_read, 
                     uint32_t & transfered)
{
    return Read(buffer, size_to_read, -1, transfered);
}

bool NamedPipe::Read(void * buffer, uint32_t size_to_read, uint32_t timeout, 
                     uint32_t & transfered)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(buffer == 0)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    NamedPipeAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.SetBuffer(buffer, size_to_read);
    
    if(!ReadAsync(args))
        return false;

    if(!complete_event.Wait(timeout))
        Cancel();

    if(!WaitNamedPipeAsyncEvent(args))
        return false;

    transfered = args.transfered();
    return true;
}

bool NamedPipe::ReadAsync(NamedPipeAsyncContext & args)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(args.data() == 0)
        return false;

    args.last_op_ = AsyncNamedPipeOp::kAsyncPipeRead;

    BOOL comp_synch = FALSE;
    if(args.completion_delegate_ != 0 && io_handler_ == 0)
    {
        auto iocr = NamedPipeRoutines::OnCompleted;
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

bool NamedPipe::Write(const void * buffer, uint32_t size_to_write, 
                      uint32_t & transfered)
{
    return Write(buffer, size_to_write, -1, transfered);
}

bool NamedPipe::Write(const void * buffer, uint32_t size_to_write, 
                      uint32_t timeout, uint32_t & transfered)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(buffer == 0)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    NamedPipeAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.SetBuffer(buffer, size_to_write);
    
    if(!WriteAsync(args))
        return false;

    if(!complete_event.Wait(timeout))
        Cancel();

    if(!WaitNamedPipeAsyncEvent(args))
        return false;

    transfered = args.transfered();
    return true;
}

bool NamedPipe::WriteAsync(NamedPipeAsyncContext & args)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(args.data() == 0)
        return false;

    args.last_op_ = AsyncNamedPipeOp::kAsyncPipeWrite;

    BOOL comp_synch = FALSE;
    if(args.completion_delegate_ != 0 && io_handler_ == 0)
    {
        auto iocr = NamedPipeRoutines::OnCompleted;
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

bool NamedPipe::Peek(void * buffer, 
                     uint32_t size_to_read, 
                     uint32_t & transfered, 
                     uint32_t & bytes_avail,
                     uint32_t & bytes_left_this_message)
{
    DWORD readsize = 0;
    DWORD ba = 0;
    DWORD bltm = 0;
    
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(!PeekNamedPipe(handle_, buffer, size_to_read, &readsize, &ba, &bltm))
        return false;

    bytes_avail = ba;
    bytes_left_this_message = bltm;
    transfered = readsize;
    return true;
}

bool NamedPipe::Cancel()
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;
    return ::CancelIo(handle_) != FALSE;
}

bool NamedPipe::Associate(Proactor & io)
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
        
void * NamedPipe::GetPlatformHandle()
{
    return handle_;
}

void NamedPipe::OnCompleted(AsyncContext & args,
                            uint32_t error,
                            uint32_t transfered)
{
    auto & named_pipe_args = static_cast<NamedPipeAsyncContext&>(args);
    named_pipe_args.OnCompleted(error, transfered);
}

bool NamedPipe::IsValid() const
{
    return handle_ != INVALID_HANDLE_VALUE;
}

bool NamedPipe::WaitNamedPipeAsyncEvent(NamedPipeAsyncContext & args)
{
    bool succeed = true;
    DWORD error = 0;
    DWORD transed = 0;

    if(!GetOverlappedResult(handle_, &args.overlapped_, &transed, TRUE))
    {
        error = GetLastError();
        succeed = false;
    }
    OnCompleted(args, error, transed);
    return succeed;
}

//NamedPipeServer

NamedPipeServer::NamedPipeServer()
{
}

NamedPipeServer::~NamedPipeServer()
{
    fini();
}

NamedPipeServer::NamedPipeServer(NamedPipeServer && obj)
{
    std::swap(handle_, obj.handle_);
}

NamedPipeServer & NamedPipeServer::operator = (NamedPipeServer && obj)
{
    std::swap(handle_, obj.handle_);
    return *this;
}

bool NamedPipeServer::init(const char * pipe_name, 
                           PipeDirection direction,
                           PipeOption option,
                           PipeTransmissionMode transmission,
                           uint32_t max_instances,
                           uint32_t out_buffer_size,
                           uint32_t in_buffer_size,
                           uint32_t timeout)
{

    uint32_t pmf = transmission;
    uint32_t omf = direction | option | FILE_FLAG_OVERLAPPED;

    wchar_t pipe_name16[kMaxPath16];
    if(!UTF8::Decode(pipe_name, -1, pipe_name16, kMaxPath16))
        return false;
        
    handle_= CreateNamedPipe(pipe_name16, omf, pmf, max_instances, 
                             out_buffer_size, in_buffer_size, timeout, 0);

    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    return true;
}


void NamedPipeServer::fini()
{
    auto sh = handle_;
    if(sh != INVALID_HANDLE_VALUE)
    {
        handle_ = INVALID_HANDLE_VALUE;
        CloseHandle(sh);
    }
}

bool NamedPipeServer::Accept()
{
    return Accept(-1);
}

bool NamedPipeServer::Accept(uint32_t timeout)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    NamedPipeAsyncContext args(complete_event);
    args.SuppressIOCP();
    if(!AcceptAsync(args))
        return false;

    if(!complete_event.Wait(timeout))
        Cancel();

    if(!WaitNamedPipeAsyncEvent(args))
        return false;

    return true;
}

bool NamedPipeServer::AcceptAsync(NamedPipeAsyncContext & args)
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    args.last_op_ = AsyncNamedPipeOp::kAsyncPipeAccept;

    if(!ConnectNamedPipe(handle_, &args.overlapped_))
    {
        uint32_t last_err = GetLastError();
        switch(last_err)
        {
        case ERROR_PIPE_CONNECTED:
            if(args.overlapped_.hEvent)
                ::SetEvent(args.overlapped_.hEvent);
        case ERROR_IO_PENDING:
            return true;
        default:
            return false;
        }
    }
    return true;
}

bool NamedPipeServer::Disconnect()
{
    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    if(::DisconnectNamedPipe(handle_))
        if(::GetLastError() != ERROR_PIPE_NOT_CONNECTED)
            return true;
    
    return false;
}


//NamePipeClient
NamedPipeClient::NamedPipeClient()
{
}

NamedPipeClient::~NamedPipeClient()
{
    fini();
}

NamedPipeClient::NamedPipeClient(NamedPipeClient && obj)
{
    std::swap(handle_, obj.handle_);
}

NamedPipeClient & NamedPipeClient::operator = (NamedPipeClient && obj)
{
    std::swap(handle_, obj.handle_);
    return *this;
}

bool NamedPipeClient::init(const char * pipe_name,
                           PipeDirection direction,
                           PipeOption option,
                           uint32_t timeout)
{

    DWORD access = 0;

    switch(direction)
    {
    case PipeDirection::kIn:
        access = GENERIC_READ;
        break;
    case PipeDirection::kOut:
        access = GENERIC_WRITE;
        break;
    case PipeDirection::kDuplex:
        access = GENERIC_READ | GENERIC_WRITE;
        break;
    default:
        return false;
    }

    wchar_t pipe_name16[kMaxPath16];
    if(!UTF8::Decode(pipe_name, -1, pipe_name16, kMaxPath16))
        return false;
    
    handle_ = CreateFile(pipe_name16, access, 0, 0, OPEN_EXISTING, 
                         option | FILE_FLAG_OVERLAPPED, 0);

    if(handle_ == INVALID_HANDLE_VALUE)
        return false;

    return true;
}

void NamedPipeClient::fini()
{
    auto sh = handle_;
    if(sh != INVALID_HANDLE_VALUE)
    {
        handle_ = INVALID_HANDLE_VALUE;
        CloseHandle(sh);
    }
}


}
