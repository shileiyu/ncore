#include <gtest\gtest.h>
#include <ncore/sys/thread.h>
#include <ncore/sys/named_pipe.h>
#include <ncore/sys/proactor.h>
#include <ncore/sys/named_pipe_async_event_args.h>
#include <ncore/algorithm/md5.h>
#include <ncore/utils/handy.h>


namespace 
{

using namespace ncore;

// 计算哈希
static void CalcHash(const void * data, size_t size, Hash & hash)
{
    MD5Provider md5_provider;
    md5_provider.Reset();
    md5_provider.Update(data, size);
    md5_provider.Final(hash);
}

// SyncStreamEchoServer
class SyncStreamEchoServer
{
public:
    SyncStreamEchoServer();
    ~SyncStreamEchoServer();

    bool init(const std::string & pipe_name, 
              uint32_t max_instances,
              uint32_t out_buffer_size,
              uint32_t in_buffer_size,
              uint32_t timeout);
    void fini();

    bool is_running() const;

private:
    void IOProc();

private:
    bool is_running_;

    NamedPipeServer pipe_;
    Thread io_thread_;
    ThreadProcAdapter<SyncStreamEchoServer> io_proc_;
};

SyncStreamEchoServer::SyncStreamEchoServer()
    : is_running_(false)
{
    io_proc_.Register(this, &SyncStreamEchoServer::IOProc);
}

SyncStreamEchoServer::~SyncStreamEchoServer()
{
}

bool SyncStreamEchoServer::init(const std::string & pipe_name, 
                                uint32_t max_instances,
                                uint32_t out_buffer_size,
                                uint32_t in_buffer_size,
                                uint32_t timeout)
{
    bool succeed = false;

    succeed = pipe_.init(pipe_name.data(), 
                         PipeDirection::kDuplex, 
                         PipeOption::kNone, 
                         PipeTransmissionMode::kStream, 
                         max_instances, 
                         out_buffer_size, 
                         in_buffer_size, 
                         timeout);
    if (!succeed) return false;

    if(!io_thread_.init(io_proc_))
        return false;

    is_running_ = true;

    if(!io_thread_.Start())
        return false;

    return true;
}

void SyncStreamEchoServer::fini()
{
    pipe_.Disconnect();
    pipe_.fini();

    is_running_ = false;
    io_thread_.fini();
}

bool SyncStreamEchoServer::is_running() const
{
    return is_running_;
}

void SyncStreamEchoServer::IOProc()
{
    while (is_running_)
    {
        if (!pipe_.Accept())
        {
            Sleep(1);
            continue;
        }

        bool succeed = false;
        while (is_running_)
        {
            char buffer[1024] = {0};
            uint32_t recv_size = 0;
            succeed = pipe_.Read(buffer, sizeof(buffer), recv_size);
            if (!succeed) break;
            if (recv_size == 0)
            {
                pipe_.Disconnect();
                break;
            }

            uint32_t send_size = 0;
            succeed = pipe_.Write(buffer, recv_size, send_size);
            if (!succeed) break;
            if (send_size == 0)
            {
                pipe_.Disconnect();
                break;
            }
        }
    }
}

// SyncMessageEchoServer
class SyncMessageEchoServer
{
public:
    SyncMessageEchoServer();
    ~SyncMessageEchoServer();

    bool init(const std::string & pipe_name, 
              uint32_t max_instances,
              uint32_t out_buffer_size,
              uint32_t in_buffer_size,
              uint32_t timeout);
    void fini();

    bool is_running() const;

private:
    void IOProc();

private:
    bool is_running_;

    NamedPipeServer pipe_;

    Thread io_thread_;
    ThreadProcAdapter<SyncMessageEchoServer> io_proc_;
};

SyncMessageEchoServer::SyncMessageEchoServer()
    : is_running_(false)
{
    io_proc_.Register(this, &SyncMessageEchoServer::IOProc);
}

SyncMessageEchoServer::~SyncMessageEchoServer()
{
}

bool SyncMessageEchoServer::init(const std::string & pipe_name, 
                                 uint32_t max_instances,
                                 uint32_t out_buffer_size,
                                 uint32_t in_buffer_size,
                                 uint32_t timeout)
{
    bool succeed = false;

    succeed = pipe_.init(pipe_name.data(), 
                         PipeDirection::kDuplex, 
                         PipeOption::kNone, 
                         PipeTransmissionMode::kMessage, 
                         max_instances, 
                         out_buffer_size, 
                         in_buffer_size, 
                         timeout);
    if (!succeed) return false;

    if(!io_thread_.init(io_proc_))
        return false;

    is_running_ = true;

    if(!io_thread_.Start())
        return false;

    return true;
}

void SyncMessageEchoServer::fini()
{
    pipe_.Disconnect();
    pipe_.fini();

    is_running_ = false;
    io_thread_.fini();
}

bool SyncMessageEchoServer::is_running() const
{
    return is_running_;
}

void SyncMessageEchoServer::IOProc()
{
    while (is_running_)
    {
        bool succeed = false;

        uint32_t trans = 0;
        uint32_t avail = 0;
        uint32_t left_this_message = 0;

        succeed = pipe_.Peek(NULL, 0, trans, avail, left_this_message);
        if (!succeed || avail <= 0)
        {
            Sleep(1);
            continue;
        }

        char buffer[204800] = {0};  // 200KB
        uint32_t recv_size = 0;
        succeed = pipe_.Read(buffer, sizeof(buffer), recv_size);
        if (!succeed) continue;
        if (recv_size == 0)
        {
            pipe_.Disconnect();
            continue;
        }

        uint32_t send_size = 0;
        succeed = pipe_.Write(buffer, recv_size, send_size);
        if (!succeed) continue;
        if (send_size == 0)
        {
            pipe_.Disconnect();
            continue;
        }
    }
}

// 异步操作的ClientContext
class ClientContext
{
public:
    static const size_t kMaxBufferSize = 204800;    // 200KB

public:
    ClientContext();
    ~ClientContext();

    NamedPipeAsyncContext & arg();
    char * buffer();

private:
    NamedPipeAsyncContext arg_;
    char buffer_[kMaxBufferSize];
};

ClientContext::ClientContext()
{
}

ClientContext::~ClientContext()
{
}

NamedPipeAsyncContext & ClientContext::arg()
{
    return arg_;
}

char * ClientContext::buffer()
{
    return buffer_;
}

// AsyncStreamEchoServer
class AsyncStreamEchoServer
{
public:
    static const size_t kMaxAccept = 1;

public:
    AsyncStreamEchoServer();
    ~AsyncStreamEchoServer();

    bool init(const std::string & pipe_name, 
              uint32_t max_instances,
              uint32_t out_buffer_size,
              uint32_t in_buffer_size,
              uint32_t timeout);
    void fini();

    bool is_running() const;

private:
    bool StartAccept();

    void IOProc();

    void OnIOCompleted(NamedPipeAsyncContext & arg);
    void ProcessAccept(NamedPipeAsyncContext & arg);
    void ProcessRead(NamedPipeAsyncContext & arg);
    void ProcessWrite(NamedPipeAsyncContext & arg);

    ClientContext & GetClientContext(NamedPipeAsyncContext & arg);
    void DisconnectClient(ClientContext & client_context);

private:
    Proactor proactor_;
    NamedPipeAsyncResultAdapter<AsyncStreamEchoServer> event_adapter_;

    Thread io_thread_;
    ThreadProcAdapter<AsyncStreamEchoServer> io_proc_;

    NamedPipeServer server_pipe_;
    ClientContext client_context_[kMaxAccept];

    bool is_running_;
    size_t io_count_;
};


AsyncStreamEchoServer::AsyncStreamEchoServer()
    : is_running_(false)
    , io_count_(0)
{
    io_proc_.Register(this, &AsyncStreamEchoServer::IOProc);
    event_adapter_.Register(this, &AsyncStreamEchoServer::OnIOCompleted);
}

AsyncStreamEchoServer::~AsyncStreamEchoServer()
{
}

bool AsyncStreamEchoServer::init(const std::string & pipe_name, 
                                 uint32_t max_instances,
                                 uint32_t out_buffer_size,
                                 uint32_t in_buffer_size,
                                 uint32_t timeout)
{
    bool succeed = false;

    succeed = proactor_.init();
    if(!succeed) return false;

    succeed = server_pipe_.init(pipe_name.data(), 
                                PipeDirection::kDuplex, 
                                PipeOption::kNone, 
                                PipeTransmissionMode::kStream, 
                                max_instances, 
                                out_buffer_size, 
                                in_buffer_size, 
                                timeout);
    if(!succeed) return false;

    succeed = server_pipe_.Associate(proactor_);
    if(!succeed) return false;

    succeed = StartAccept();
    if(!succeed) return false;

    succeed = io_thread_.init(io_proc_);
    if(!succeed) return false;

    is_running_ = true;

    succeed = io_thread_.Start();
    if(!succeed) return false;

    return true;
}

void AsyncStreamEchoServer::fini()
{
    server_pipe_.Disconnect();
    server_pipe_.fini();

    while (io_count_) Sleep(1);

    is_running_ = false;
    io_thread_.fini();

    proactor_.fini();
}

bool AsyncStreamEchoServer::is_running() const
{
    return is_running_;
}

bool AsyncStreamEchoServer::StartAccept()
{
    bool succeed = false;

    for (size_t index = 0; index < kMaxAccept; ++index)
    {
        ClientContext & client_context = client_context_[index];

        NamedPipeAsyncContext & arg = client_context.arg();
        arg.set_user_token(&client_context);
        arg.set_completion_delegate(&event_adapter_);

        succeed = server_pipe_.AcceptAsync(arg);
        if (!succeed)
            return false;
        else
            ++io_count_;
    }

    return true;
}

void AsyncStreamEchoServer::IOProc()
{
    while (is_running_) proactor_.Run(1);
}

void AsyncStreamEchoServer::OnIOCompleted(NamedPipeAsyncContext & arg)
{
    switch (arg.last_op())
    {
    case AsyncNamedPipeOp::kAsyncPipeAccept:
        {
            ProcessAccept(arg);
        }
        break;
    case AsyncNamedPipeOp::kAsyncPipeRead:
        {
            ProcessRead(arg);
        }
        break;
    case AsyncNamedPipeOp::kAsyncPipeWrite:
        {
            ProcessWrite(arg);
        }
        break;
    }

    --io_count_;
}

void AsyncStreamEchoServer::ProcessAccept(NamedPipeAsyncContext & arg)
{
    ClientContext & client_context = GetClientContext(arg);

    if (arg.error())
    {
        DisconnectClient(client_context);
        return ;
    }

    arg.SetBuffer(client_context.buffer(), ClientContext::kMaxBufferSize);
    bool succeed = server_pipe_.ReadAsync(arg);
    if (!succeed)
        DisconnectClient(client_context);
    else
        ++io_count_;
}

void AsyncStreamEchoServer::ProcessRead(NamedPipeAsyncContext & arg)
{
    ClientContext & client_context = GetClientContext(arg);

    if (arg.error() || arg.transfered() <= 0)
    {
        DisconnectClient(client_context);
        return ;
    }

    arg.SetBuffer(client_context.buffer(), arg.transfered());
    bool succeed = server_pipe_.WriteAsync(arg);
    if (!succeed)
        DisconnectClient(client_context);
    else
        ++io_count_;
}

void AsyncStreamEchoServer::ProcessWrite(
    NamedPipeAsyncContext & arg
) {
    ClientContext & client_context = GetClientContext(arg);

    if (arg.error() || arg.transfered() <= 0)
    {
        DisconnectClient(client_context);
        return ;
    }

    size_t size_to_send = arg.count() - arg.transfered();
    if (size_to_send)
    {
        void * dst = reinterpret_cast<char *>(arg.data()) + arg.transfered();
        arg.SetBuffer(dst, size_to_send);
        bool succeed = server_pipe_.WriteAsync(arg);
        if (!succeed)
            DisconnectClient(client_context);
        else
            ++io_count_;
        return ;
    }

    arg.SetBuffer(client_context.buffer(), ClientContext::kMaxBufferSize);
    bool succeed = server_pipe_.ReadAsync(arg);
    if (!succeed)
        DisconnectClient(client_context);
    else
        ++io_count_;
}

ClientContext & AsyncStreamEchoServer::GetClientContext(
    NamedPipeAsyncContext & arg
) {
    return *reinterpret_cast<ClientContext *>(arg.user_token());
}

void AsyncStreamEchoServer::DisconnectClient(ClientContext & client_context)
{
    bool succeed = false;

    succeed  = server_pipe_.Disconnect();
    if (!succeed) return ;

    client_context.arg().SetBuffer(client_context.buffer(), 0);
    succeed = server_pipe_.AcceptAsync(client_context.arg());
    if (!succeed)
        return ;
    else
        ++io_count_;
}

// AsyncMessageEchoServer
class AsyncMessageEchoServer
{
public:
    static const size_t kMaxAccept = 1;

public:
    AsyncMessageEchoServer();
    ~AsyncMessageEchoServer();

    bool init(const std::string & pipe_name, 
              uint32_t max_instances,
              uint32_t out_buffer_size,
              uint32_t in_buffer_size,
              uint32_t timeout);
    void fini();

    bool is_running() const;

private:
    bool StartAccept();

    void IOProc();

    void OnIOCompleted(NamedPipeAsyncContext & arg);
    void ProcessAccept(NamedPipeAsyncContext & arg);
    void ProcessRead(NamedPipeAsyncContext & arg);
    void ProcessWrite(NamedPipeAsyncContext & arg);

    ClientContext & GetClientContext(NamedPipeAsyncContext & arg);
    void DisconnectClient(ClientContext & client_context);

private:
    Proactor proactor_;
    NamedPipeAsyncResultAdapter<AsyncMessageEchoServer> event_adapter_;
    Thread io_thread_;
    ThreadProcAdapter<AsyncMessageEchoServer> io_proc_;
    NamedPipeServer server_pipe_;
    ClientContext client_context_[kMaxAccept];

    bool is_running_;
    size_t io_count_;
};


AsyncMessageEchoServer::AsyncMessageEchoServer()
    : is_running_(false)
    , io_count_(0)
{
    io_proc_.Register(this, &AsyncMessageEchoServer::IOProc);
    event_adapter_.Register(this, &AsyncMessageEchoServer::OnIOCompleted);
}

AsyncMessageEchoServer::~AsyncMessageEchoServer()
{
}

bool AsyncMessageEchoServer::init(const std::string & pipe_name, 
                                 uint32_t max_instances,
                                 uint32_t out_buffer_size,
                                 uint32_t in_buffer_size,
                                 uint32_t timeout)
{
    bool succeed = false;

    succeed = proactor_.init();
    if(!succeed) return false;

    succeed = server_pipe_.init(pipe_name.data(),
                                PipeDirection::kDuplex,
                                PipeOption::kNone, 
                                PipeTransmissionMode::kMessage, 
                                max_instances, 
                                out_buffer_size, 
                                in_buffer_size, 
                                timeout);
    if(!succeed) return false;

    succeed = server_pipe_.Associate(proactor_);
    if(!succeed) return false;

    succeed = StartAccept();
    if(!succeed) return false;

    succeed = io_thread_.init(io_proc_);
    if(!succeed) return false;

    is_running_ = true;

    succeed = io_thread_.Start();
    if(!succeed) return false;

    return true;
}

void AsyncMessageEchoServer::fini()
{
    server_pipe_.Disconnect();
    server_pipe_.fini();

    while (io_count_) Sleep(1);

    is_running_ = false;
    io_thread_.fini();

    proactor_.fini();
}

bool AsyncMessageEchoServer::is_running() const
{
    return is_running_;
}

bool AsyncMessageEchoServer::StartAccept()
{
    bool succeed = false;

    for (size_t index = 0; index < kMaxAccept; ++index)
    {
        ClientContext & client_context = client_context_[index];

        NamedPipeAsyncContext & arg = client_context.arg();
        arg.set_user_token(&client_context);
        arg.set_completion_delegate(&event_adapter_);

        succeed = server_pipe_.AcceptAsync(arg);
        if (!succeed)
            return false;
        else
            ++io_count_;
    }

    return true;
}

void AsyncMessageEchoServer::IOProc()
{
    while (is_running_) proactor_.Run(1);
}

void AsyncMessageEchoServer::OnIOCompleted(
    NamedPipeAsyncContext & arg)
{
    switch (arg.last_op())
    {
    case AsyncNamedPipeOp::kAsyncPipeAccept:
        {
            ProcessAccept(arg);
        }
        break;
    case AsyncNamedPipeOp::kAsyncPipeRead:
        {
            ProcessRead(arg);
        }
        break;
    case AsyncNamedPipeOp::kAsyncPipeWrite:
        {
            ProcessWrite(arg);
        }
        break;
    }

    --io_count_;
}

void AsyncMessageEchoServer::ProcessAccept(
    NamedPipeAsyncContext & arg)
{
    ClientContext & client_context = GetClientContext(arg);

    if (arg.error())
    {
        DisconnectClient(client_context);
        return ;
    }

    arg.SetBuffer(client_context.buffer(), ClientContext::kMaxBufferSize);
    bool succeed = server_pipe_.ReadAsync(arg);
    if (!succeed)
        DisconnectClient(client_context);
    else
        ++io_count_;
}

void AsyncMessageEchoServer::ProcessRead(
    NamedPipeAsyncContext & arg)
{
    if (arg.error() || arg.transfered() <= 0) return ;

    ClientContext & client_context = GetClientContext(arg);

    arg.SetBuffer(client_context.buffer(), arg.transfered());
    bool succeed = server_pipe_.WriteAsync(arg);
    if (!succeed)
        return ;
    else
        ++io_count_;
}

void AsyncMessageEchoServer::ProcessWrite(
    NamedPipeAsyncContext & arg)
{
    ClientContext & client_context = GetClientContext(arg);

    arg.SetBuffer(client_context.buffer(), ClientContext::kMaxBufferSize);
    bool succeed = server_pipe_.ReadAsync(arg);
    if (!succeed)
        return ;
    else
        ++io_count_;
}

ClientContext & AsyncMessageEchoServer::GetClientContext(
    NamedPipeAsyncContext & arg)
{
    return *reinterpret_cast<ClientContext *>(arg.user_token());
}

void AsyncMessageEchoServer::DisconnectClient(ClientContext & client_context)
{
    bool succeed = false;

    succeed  = server_pipe_.Disconnect();
    if (!succeed) return ;

    client_context.arg().SetBuffer(client_context.buffer(), 0);
    succeed = server_pipe_.AcceptAsync(client_context.arg());
    if (!succeed)
        return ;
    else
        ++io_count_;
}

// AsyncStreamClient
class AsyncStreamClient
{
public:
    static const size_t kMaxBufferSize = 204800;    // 200KB

public:
    AsyncStreamClient();
    ~AsyncStreamClient();

    bool init(const std::string & pipe_name, 
              uint32_t timeout);
    void fini();

    bool is_connected() const;
    bool is_completed() const;

    void SetSendBuffer(const void * buffer, size_t size);

    const void * send_buffer();
    size_t send_buffer_size();

    const void * recv_buffer();
    size_t recv_buffer_size();

    bool StartWrite();

    void IOProc();

private:
    void OnIOCompleted(NamedPipeAsyncContext & arg);
    void ProcessWrite(NamedPipeAsyncContext & arg);
    void ProcessRead(NamedPipeAsyncContext & arg);

    void Disconnect();

private:
    Proactor proactor_;
    NamedPipeAsyncResultAdapter<AsyncStreamClient> event_adapter_;

    NamedPipeClient client_pipe_;
    ClientContext client_context_;

    char send_buffer_[kMaxBufferSize];
    char recv_buffer_[kMaxBufferSize];
    size_t send_buffer_size_;
    size_t recv_buffer_size_;
    size_t size_sended_;

    bool is_connected_;
    bool is_completed_;
    size_t io_count_;
};

AsyncStreamClient::AsyncStreamClient()
    : send_buffer_size_(0)
    , recv_buffer_size_(0)
    , size_sended_(0)
    , is_connected_(false)
    , is_completed_(false)
    , io_count_(0)
{
    event_adapter_.Register(this, &AsyncStreamClient::OnIOCompleted);
}

AsyncStreamClient::~AsyncStreamClient()
{
}

bool AsyncStreamClient::init(const std::string & pipe_name, 
                             uint32_t timeout)
{
    bool succeed = false;

    succeed = proactor_.init();
    if(!succeed) return false;

    client_pipe_.init(pipe_name.data(), 
                      PipeDirection::kDuplex, 
                      PipeOption::kNone, 
                      timeout);

    succeed = client_pipe_.Associate(proactor_);
    if(!succeed) return false;

    is_connected_ = true;

    return true;
}

void AsyncStreamClient::fini()
{
    client_pipe_.fini();

    while (io_count_) IOProc();

    is_connected_ = false;
    is_completed_ = true;

    proactor_.fini();
}

bool AsyncStreamClient::is_connected() const
{
    return is_connected_;
}

bool AsyncStreamClient::is_completed() const
{
    return is_completed_;
}

void AsyncStreamClient::SetSendBuffer(const void * buffer, size_t size)
{
    memset(send_buffer_, 0, kMaxBufferSize);
    send_buffer_size_ = kMaxBufferSize < size ? kMaxBufferSize : size;
    memcpy(send_buffer_, buffer, send_buffer_size_);
}

const void * AsyncStreamClient::send_buffer()
{
    return send_buffer_;
}

size_t AsyncStreamClient::send_buffer_size()
{
    return send_buffer_size_;
}

const void * AsyncStreamClient::recv_buffer()
{
    return recv_buffer_;
}

size_t AsyncStreamClient::recv_buffer_size()
{
    return recv_buffer_size_;
}

bool AsyncStreamClient::StartWrite()
{
    is_completed_ = false;

    memset(recv_buffer_, 0, kMaxBufferSize);
    recv_buffer_size_ = 0;
    size_sended_ = 0;

    NamedPipeAsyncContext & arg = client_context_.arg();
    arg.SetBuffer(send_buffer_, send_buffer_size_);
    arg.set_completion_delegate(&event_adapter_);

    bool succeed = client_pipe_.WriteAsync(arg);
    if (!succeed)
        return false;
    else
        ++io_count_;

    return true;
}

void AsyncStreamClient::IOProc()
{
    proactor_.Run(1);
}

void AsyncStreamClient::OnIOCompleted(NamedPipeAsyncContext & arg)
{
    switch (arg.last_op())
    {
    case AsyncNamedPipeOp::kAsyncPipeWrite:
        {
            ProcessWrite(arg);
        }
        break;
    case AsyncNamedPipeOp::kAsyncPipeRead:
        {
            ProcessRead(arg);
        }
        break;
    }

    --io_count_;
}

void AsyncStreamClient::ProcessWrite(NamedPipeAsyncContext & arg)
{
    if (arg.error() || arg.transfered() <= 0)
    {
        Disconnect();
        return ;
    }

    size_sended_ += arg.transfered();
    size_t send_size = send_buffer_size_ - size_sended_;
    if (send_size)
    {
        arg.SetBuffer(send_buffer_ + size_sended_, send_size);
        bool succeed = client_pipe_.WriteAsync(arg);
        if (!succeed)
            Disconnect();
        else
            ++io_count_;
    }

    arg.SetBuffer(recv_buffer_ + recv_buffer_size_, arg.transfered());
    bool succeed = client_pipe_.ReadAsync(arg);
    if (!succeed)
        Disconnect();
    else
        ++io_count_;
}

void AsyncStreamClient::ProcessRead(NamedPipeAsyncContext & arg)
{
    if (arg.error() || arg.transfered() <= 0)
    {
        Disconnect();
        return ;
    }

    is_completed_ = false;

    recv_buffer_size_ += arg.transfered();
    size_t recv_size = send_buffer_size_ - recv_buffer_size_;
    if (recv_size)
    {
        arg.SetBuffer(recv_buffer_ + recv_buffer_size_, recv_size);
        bool succeed = client_pipe_.ReadAsync(arg);
        if (!succeed)
            Disconnect();
        else
            ++io_count_;
    }
    else
    {
        is_completed_ = true;
    }
}

void AsyncStreamClient::Disconnect()
{
    client_pipe_.fini();
}

// AsyncMessageClient
class AsyncMessageClient
{
public:
    static const size_t kMaxBufferSize = 204800;    // 200KB

public:
    AsyncMessageClient();
    ~AsyncMessageClient();

    bool init(const std::string & pipe_name, 
              uint32_t timeout);
    void fini();

    bool is_connected() const;
    bool is_completed() const;

    void SetSendBuffer(const void * buffer, size_t size);

    const void * send_buffer();
    size_t send_buffer_size();

    const void * recv_buffer();
    size_t recv_buffer_size();

    bool StartWrite();

    void IOProc();

private:
    void OnIOCompleted(NamedPipeAsyncContext & arg);
    void ProcessWrite(NamedPipeAsyncContext & arg);
    void ProcessRead(NamedPipeAsyncContext & arg);

    void Disconnect();

private:
    Proactor proactor_;
    NamedPipeAsyncResultAdapter<AsyncMessageClient> event_adapter_;

    NamedPipeClient client_pipe_;
    ClientContext client_context_;

    char send_buffer_[kMaxBufferSize];
    char recv_buffer_[kMaxBufferSize];
    size_t send_buffer_size_;
    size_t recv_buffer_size_;
    size_t size_sended_;

    bool is_connected_;
    bool is_completed_;
    size_t io_count_;
};

AsyncMessageClient::AsyncMessageClient()
    : send_buffer_size_(0)
    , recv_buffer_size_(0)
    , size_sended_(0)
    , is_connected_(false)
    , is_completed_(false)
    , io_count_(0)
{
    event_adapter_.Register(this, &AsyncMessageClient::OnIOCompleted);
}

AsyncMessageClient::~AsyncMessageClient()
{
}

bool AsyncMessageClient::init(const std::string & pipe_name, 
                             uint32_t timeout)
{
    bool succeed = false;

    succeed = proactor_.init();
    if(!succeed) return false;

    client_pipe_.init(pipe_name.data(),
                      PipeDirection::kDuplex, 
                      PipeOption::kNone, 
                      timeout);

    succeed = client_pipe_.Associate(proactor_);
    if(!succeed) return false;

    is_connected_ = true;

    return true;
}

void AsyncMessageClient::fini()
{
    while (io_count_) IOProc();

    is_connected_ = false;
    is_completed_ = true;

    proactor_.fini();
    client_pipe_.fini();
}

bool AsyncMessageClient::is_connected() const
{
    return is_connected_;
}

bool AsyncMessageClient::is_completed() const
{
    return is_completed_;
}

void AsyncMessageClient::SetSendBuffer(const void * buffer, size_t size)
{
    memset(send_buffer_, 0, kMaxBufferSize);
    send_buffer_size_ = kMaxBufferSize < size ? kMaxBufferSize : size;
    memcpy(send_buffer_, buffer, send_buffer_size_);
}

const void * AsyncMessageClient::send_buffer()
{
    return send_buffer_;
}

size_t AsyncMessageClient::send_buffer_size()
{
    return send_buffer_size_;
}

const void * AsyncMessageClient::recv_buffer()
{
    return recv_buffer_;
}

size_t AsyncMessageClient::recv_buffer_size()
{
    return recv_buffer_size_;
}

bool AsyncMessageClient::StartWrite()
{
    is_completed_ = false;

    memset(recv_buffer_, 0, kMaxBufferSize);
    recv_buffer_size_ = 0;
    size_sended_ = 0;

    NamedPipeAsyncContext & arg = client_context_.arg();
    arg.SetBuffer(send_buffer_, send_buffer_size_);
    arg.set_completion_delegate(&event_adapter_);

    bool succeed = client_pipe_.WriteAsync(arg);
    if (!succeed)
        return false;
    else
        ++io_count_;

    return true;
}

void AsyncMessageClient::IOProc()
{
    proactor_.Run(1);
}

void AsyncMessageClient::OnIOCompleted(NamedPipeAsyncContext & arg)
{
    switch (arg.last_op())
    {
    case AsyncNamedPipeOp::kAsyncPipeWrite:
        {
            ProcessWrite(arg);
        }
        break;
    case AsyncNamedPipeOp::kAsyncPipeRead:
        {
            ProcessRead(arg);
        }
        break;
    }

    --io_count_;
}

void AsyncMessageClient::ProcessWrite(NamedPipeAsyncContext & arg)
{
    is_completed_ = false;

    if (arg.error() || arg.transfered() < send_buffer_size_)
    {
        is_completed_ = true;
        return ;
    }

    arg.SetBuffer(recv_buffer_, arg.transfered());
    bool succeed = client_pipe_.ReadAsync(arg);
    if (!succeed)
        return ;
    else
        ++io_count_;
}

void AsyncMessageClient::ProcessRead(NamedPipeAsyncContext & arg)
{
    is_completed_ = true;

    if (arg.error() || arg.transfered() < send_buffer_size_) return ;

    recv_buffer_size_ = arg.transfered();
}

void AsyncMessageClient::Disconnect()
{
    client_pipe_.fini();
}

// NamedPipeTest
class NamedPipeTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        for (size_t index = 0; index < countof(send_buffer_); ++index)
            send_buffer_[index] = GetTickCount();
    }

    static void TearDownTestCase()
    {
    }

protected:

    static int send_buffer_[51200];  // 200KB
};

int NamedPipeTest::send_buffer_[51200];

// 同步Stream测试
TEST_F(NamedPipeTest, SyncStreamServerSyncClient)
{
    bool succeed = false;

    std::string pipe_name("\\\\.\\pipe\\sync_stream_server");

    SyncStreamEchoServer sync_stream_echo_server;
    succeed = sync_stream_echo_server.init(pipe_name, 
                                           1,
                                           204800,
                                           204800,
                                           1000);
    ASSERT_TRUE(succeed);

    // SyncStreamEchoServer is running
    succeed = sync_stream_echo_server.is_running();
    ASSERT_TRUE(succeed);

    // Client init and Connect to SyncStreamEchoServer
    NamedPipeClient client;
    succeed = client.init(pipe_name.data(), 
                          PipeDirection::kDuplex, 
                          PipeOption::kNone, 
                          1000);
    ASSERT_TRUE(succeed);

    // Prepare buffer
    // 1KB, 32KB, 64KB, 75KB, 200KB
    int send_buffer_size[] = {1024, 32768, 65536, 76800, 204800};

    for (size_t count = 0; count < countof(send_buffer_size); ++count)
    {
        // Send buffer to SyncStreamEchoServer
        const uint32_t size_to_send = send_buffer_size[count];
        uint32_t send_size = 0;
        succeed = client.Write(send_buffer_, size_to_send, send_size);
        EXPECT_TRUE(succeed);
        EXPECT_EQ(size_to_send, send_size);

        // Receive from SyncStreamEchoServer
        int receive_buffer_[51200] = {0};  // 200KB
        char * recv_buffer = reinterpret_cast<char *>(receive_buffer_);
        uint32_t size_to_recv = 0;
        while (size_to_recv < size_to_send)
        {
            // position and remainning size to recv
            char * dst = recv_buffer + size_to_recv;
            uint32_t dst_size = size_to_send - size_to_recv;
            uint32_t recv_size = 0;
            succeed = client.Read(dst, dst_size, recv_size);
            EXPECT_TRUE(succeed);
            if (recv_size == 0) break;
            size_to_recv += recv_size;
        }

        EXPECT_EQ(size_to_send, size_to_recv);

        // Calculate hash of send buffer
        Hash send_buffer_hash;
        CalcHash(send_buffer_, size_to_send, send_buffer_hash);

        // Calculate hash of recv buffer
        Hash recv_buffer_hash;
        CalcHash(recv_buffer, size_to_recv, recv_buffer_hash);

        EXPECT_TRUE(send_buffer_hash == recv_buffer_hash);
    }

    client.fini();
    sync_stream_echo_server.fini();
}

// 异步Stream测试
TEST_F(NamedPipeTest, AsyncStreamServerAsyncClient)
{
    bool succeed = false;

    std::string pipe_name("\\\\.\\pipe\\async_stream_server");

    AsyncStreamEchoServer async_stream_echo_server;
    succeed = async_stream_echo_server.init(pipe_name, 
                                            1,
                                            204800,
                                            204800,
                                            1000);
    ASSERT_TRUE(succeed);

    // AsyncStreamEchoServer is running
    succeed = async_stream_echo_server.is_running();
    ASSERT_TRUE(succeed);

    // Client init and Connect to AsyncStreamEchoServer
    AsyncStreamClient async_client;
    succeed = async_client.init(pipe_name, 
                                1000);
    ASSERT_TRUE(succeed);

    succeed = async_client.is_connected();
    ASSERT_TRUE(succeed);

    // Prepare buffer
    // 1KB, 32KB, 64KB, 75KB, 200KB
    int send_buffer_size[] = {1024, 32768, 65536, 76800, 204800};

    for (size_t count = 0; count < countof(send_buffer_size); ++count)
    {
        // AsyncClient set send buffer
        async_client.SetSendBuffer(send_buffer_, send_buffer_size[count]);

        succeed = async_client.StartWrite();
        EXPECT_TRUE(succeed);

        while (!async_client.is_completed()) async_client.IOProc();

        // Calculate hash of send buffer
        const void * send_buffer = async_client.send_buffer();
        const size_t size_to_send = async_client.send_buffer_size();
        Hash send_buffer_hash;
        CalcHash(send_buffer, size_to_send, send_buffer_hash);

        // Calculate hash of recv buffer
        const void * recv_buffer = async_client.recv_buffer();
        const size_t size_to_recv = async_client.recv_buffer_size();
        Hash recv_buffer_hash;
        CalcHash(recv_buffer, size_to_recv, recv_buffer_hash);

        EXPECT_TRUE(send_buffer_hash == recv_buffer_hash);
    }

    async_client.fini();
    async_stream_echo_server.fini();
}

// 同步Message测试
TEST_F(NamedPipeTest, SyncMessageServerSyncClient)
{
    bool succeed = false;

    std::string pipe_name("\\\\.\\pipe\\sync_message_server");

    SyncMessageEchoServer sync_message_echo_server;
    succeed = sync_message_echo_server.init(pipe_name, 
                                            1,
                                            204800,
                                            204800,
                                            1000);
    ASSERT_TRUE(succeed);

    // SyncMessageEchoServer is running
    succeed = sync_message_echo_server.is_running();
    ASSERT_TRUE(succeed);

    // Client init and Connect to SyncMessageEchoServer
    NamedPipeClient client;
    succeed = client.init(pipe_name.data(), 
                          PipeDirection::kDuplex, 
                          PipeOption::kNone, 
                          1000);
    ASSERT_TRUE(succeed);

    // Prepare buffer
    // 1KB, 32KB, 64KB, 75KB, 200KB
    int send_buffer_size[] = {1024, 32768, 65536, 76800, 204800};

    for (size_t count = 0; count < countof(send_buffer_size); ++count)
    {
        // Send buffer to SyncMessageEchoServer
        const uint32_t size_to_send = send_buffer_size[count];
        uint32_t send_size = 0;
        succeed = client.Write(send_buffer_, size_to_send, send_size);
        EXPECT_TRUE(succeed);
        EXPECT_EQ(size_to_send, send_size);

        // Receive from SyncMessageEchoServer
        int receive_buffer_[51200] = {0};  // 200KB
        char * recv_buffer = reinterpret_cast<char *>(receive_buffer_);
        uint32_t size_to_recv = size_to_send;
        uint32_t recv_size = 0;
        succeed = client.Read(recv_buffer, size_to_recv, recv_size);
        EXPECT_TRUE(succeed);
        EXPECT_EQ(size_to_recv, recv_size);

        // Calculate hash of send buffer
        Hash send_buffer_hash;
        CalcHash(send_buffer_, size_to_send, send_buffer_hash);

        // Calculate hash of recv buffer
        Hash recv_buffer_hash;
        CalcHash(recv_buffer, size_to_recv, recv_buffer_hash);

        EXPECT_TRUE(send_buffer_hash == recv_buffer_hash);
    }

    client.fini();
    sync_message_echo_server.fini();
}

// 异步Message测试
TEST_F(NamedPipeTest, AsyncMessageServerAsyncClient)
{
    bool succeed = false;

    std::string pipe_name("\\\\.\\pipe\\async_message_server");

    AsyncMessageEchoServer async_message_echo_server;
    succeed = async_message_echo_server.init(pipe_name, 
                                            1,
                                            204800,
                                            204800,
                                            1000);
    ASSERT_TRUE(succeed);

    // AsyncMessageEchoServer is running
    succeed = async_message_echo_server.is_running();
    ASSERT_TRUE(succeed);

    // Client init and Connect to AsyncMessageEchoServer
    AsyncMessageClient async_client;
    succeed = async_client.init(pipe_name, 
                                1000);
    ASSERT_TRUE(succeed);

    succeed = async_client.is_connected();
    ASSERT_TRUE(succeed);

    // Prepare buffer
    // 1KB, 32KB, 64KB, 75KB, 200KB
    int send_buffer_size[] = {1024, 32768, 65536, 76800, 204800};

    for (size_t count = 0; count < countof(send_buffer_size); ++count)
    {
        // AsyncClient set send buffer
        async_client.SetSendBuffer(send_buffer_, send_buffer_size[count]);

        succeed = async_client.StartWrite();
        EXPECT_TRUE(succeed);

        while (!async_client.is_completed()) async_client.IOProc();

        // Calculate hash of send buffer
        const void * send_buffer = async_client.send_buffer();
        const size_t size_to_send = async_client.send_buffer_size();
        Hash send_buffer_hash;
        CalcHash(send_buffer, size_to_send, send_buffer_hash);

        // Calculate hash of recv buffer
        const void * recv_buffer = async_client.recv_buffer();
        const size_t size_to_recv = async_client.recv_buffer_size();
        Hash recv_buffer_hash;
        CalcHash(recv_buffer, size_to_recv, recv_buffer_hash);

        EXPECT_TRUE(send_buffer_hash == recv_buffer_hash);
    }

    async_client.fini();
    async_message_echo_server.fini();
}

}