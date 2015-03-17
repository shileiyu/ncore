#include <gtest\gtest.h>
#include <ncore/sys/thread.h>
#include <ncore/sys/socket.h>
#include <ncore/sys/proactor.h>
#include <ncore/sys/socket_async_event_args.h>
#include <ncore/algorithm/md5.h>
#include <ncore/utils/handy.h>

using namespace ncore;

// 计算哈希
static void CalcHash(const void * data, size_t size, Hash & hash)
{
    MD5Provider md5_provider;
    md5_provider.Reset();
    md5_provider.Update(data, size);
    md5_provider.Final(hash);
}

// 全同步服务器（TCP已完成，UDP已完成）
class SyncEchoServer
{
public:
    static const size_t kMaxListen = 2;

public:
    SyncEchoServer();
    ~SyncEchoServer();

    bool init(const IPEndPoint & endpoint);
    void fini();

    bool is_running() const;

private:
    void TCPIOProc();
    void UDPIOProc();

private:
    // TCP
    Socket tcp_socket_;
    Thread tcp_io_thread_;
    ThreadProcAdapter<SyncEchoServer> tcp_io_proc_;

    // UDP
    Socket udp_socket_;
    Thread udp_io_thread_;
    ThreadProcAdapter<SyncEchoServer> udp_io_proc_;

    bool is_running_;
};

SyncEchoServer::SyncEchoServer()
    : is_running_(false)
{
    tcp_io_proc_.Register(this, &SyncEchoServer::TCPIOProc);
    udp_io_proc_.Register(this, &SyncEchoServer::UDPIOProc);
}

SyncEchoServer::~SyncEchoServer()
{
}

bool SyncEchoServer::init(const IPEndPoint & endpoint)
{
    if(!tcp_socket_.init(AddressFamily::kInterNetwork, 
                         SocketType::kStream, 
                         ProtocolType::kTCP) ||
       !udp_socket_.init(AddressFamily::kInterNetwork, 
                         SocketType::kDgram, 
                         ProtocolType::kUDP))
        return false;

    if(!tcp_socket_.Bind(endpoint) ||
       !udp_socket_.Bind(endpoint))
        return false;

    if(!tcp_socket_.Listen(kMaxListen))
        return false;

    if(!tcp_io_thread_.init(tcp_io_proc_) ||
       !udp_io_thread_.init(udp_io_proc_))
        return false;

    is_running_ = true;

    if(!tcp_io_thread_.Start() ||
       !udp_io_thread_.Start())
        return false;

    return true;
}

void SyncEchoServer::fini()
{
    is_running_ = false;

    udp_socket_.fini();
    tcp_socket_.fini();

    udp_io_thread_.fini();
    tcp_io_thread_.fini();
}

bool SyncEchoServer::is_running() const
{
    return is_running_;
}

void SyncEchoServer::TCPIOProc()
{
    while (is_running_)
    {
        if (!tcp_socket_.CanRead())
        {
            Sleep(1);
            continue;
        }

        Socket client = tcp_socket_.Accept();

        while (is_running_)
        {
            char recv_buffer[1024] = {0}; // 1KB

            uint32_t recv_size = 0;
            client.Receive(recv_buffer, sizeof(recv_buffer), recv_size);
            if (recv_size == 0)
            {
                client.Disconnect(false);
                client.fini();
                break;
            }

            uint32_t size_to_send = recv_size;
            uint32_t send_size = 0;
            client.Send(recv_buffer, size_to_send, send_size);
            if (send_size == 0)
            {
                client.Disconnect(false);
                client.fini();
                break;
            }
        }
    }
}

void SyncEchoServer::UDPIOProc()
{
    while (is_running_)
    {
        if (!udp_socket_.CanRead())
        {
            Sleep(1);
            continue;
        }

        IPEndPoint iep;
        char recv_buffer[65536] = {0};  // 64KB
        uint32_t recv_size = 0;
        udp_socket_.ReceiveFrom(recv_buffer, sizeof(recv_buffer), recv_size, iep);

        uint32_t size_to_send = recv_size;
        uint32_t send_size = 0;
        udp_socket_.SendTo(recv_buffer, size_to_send, iep, send_size);
    }
}

// 异步操作的ClientContext
class ClientContext
{
public:
    static const size_t kMaxBufferSize = 204800;    // 200KB
    static const size_t kMaxUDPBufferSize = 65536;  // 64KB

public:
    ClientContext();
    ~ClientContext();

    SocketAsyncContext & arg();
    Socket & socket();
    IPEndPoint & remote_endpoint();
    char * buffer();

private:
    SocketAsyncContext arg_;
    Socket socket_;
    IPEndPoint remote_endpoint_;
    char buffer_[kMaxBufferSize];
};

ClientContext::ClientContext()
{
}

ClientContext::~ClientContext()
{
}

SocketAsyncContext & ClientContext::arg()
{
    return arg_;
}

Socket & ClientContext::socket()
{
    return socket_;
}

IPEndPoint & ClientContext::remote_endpoint()
{
    return remote_endpoint_;
}

char * ClientContext::buffer()
{
    return buffer_;
}

// 全异步服务器（IOCP）（TCP已完成，UDP已完成）
class AsyncEchoServer
{
public:
    static const size_t kMaxListen = 10;

public:
    AsyncEchoServer();
    ~AsyncEchoServer();

    bool init(const IPEndPoint & endpoint);
    void fini();

    bool is_running() const;

private:
    bool StartAccept();
    bool StartReceiveFrom();

    void IOProc();

    void OnIOCompleted(SocketAsyncContext & arg);
    void ProcessAccept(SocketAsyncContext & arg);
    void ProcessRecv(SocketAsyncContext & arg);
    void ProcessSend(SocketAsyncContext & arg);
    void ProcessDisconnect(SocketAsyncContext & arg);
    void ProcessRecvFrom(SocketAsyncContext & arg);
    void ProcessSendTo(SocketAsyncContext & arg);

    ClientContext & GetClientContext(SocketAsyncContext & arg);
    void DisconnectClient(ClientContext & client_context);

private:
    Socket tcp_listen_socket_;   // TCP
    Socket udp_listen_socket_;   // UDP

    Proactor proactor_;
    SocketAsyncResultAdapter<AsyncEchoServer> event_adapter_;

    Thread io_thread_;
    ThreadProcAdapter<AsyncEchoServer> io_proc_;

    ClientContext tcp_client_context_[kMaxListen];
    ClientContext udp_client_context_[kMaxListen];

    bool is_running_;
    size_t io_count_;
};

static const size_t kAddressBufferSize  = sizeof(sockaddr_in) + 16;
static const size_t kMinAcceptBufferSize = kAddressBufferSize * 2;

AsyncEchoServer::AsyncEchoServer()
    : is_running_(false)
    , io_count_(0)
{
    io_proc_.Register(this, &AsyncEchoServer::IOProc);
    event_adapter_.Register(this, &AsyncEchoServer::OnIOCompleted);
}

AsyncEchoServer::~AsyncEchoServer()
{
}

bool AsyncEchoServer::init(const IPEndPoint & endpoint)
{
    bool succeed = false;

    succeed = tcp_listen_socket_.init(AddressFamily::kInterNetwork, 
                                      SocketType::kStream, 
                                      ProtocolType::kTCP);
    if(!succeed) return false;

    succeed = udp_listen_socket_.init(AddressFamily::kInterNetwork, 
                                      SocketType::kDgram, 
                                      ProtocolType::kUDP);
    if(!succeed) return false;

    succeed = proactor_.init();
    if(!succeed) return false;

    succeed = tcp_listen_socket_.Associate(proactor_);
    if(!succeed) return false;

    succeed = udp_listen_socket_.Associate(proactor_);
    if(!succeed) return false;

    succeed = tcp_listen_socket_.Bind(endpoint);
    if(!succeed) return false;

    succeed = udp_listen_socket_.Bind(endpoint);
    if(!succeed) return false;

    succeed = tcp_listen_socket_.Listen(kMaxListen);
    if(!succeed) return false;

    succeed = StartAccept();
    if(!succeed) return false;

    succeed = StartReceiveFrom();
    if(!succeed) return false;

    succeed = io_thread_.init(io_proc_);
    if(!succeed) return false;

    is_running_ = true;

    succeed = io_thread_.Start();
    if(!succeed) return false;

    return true;
}

void AsyncEchoServer::fini()
{
    for (size_t index = 0; index < kMaxListen; ++index)
        tcp_client_context_[index].socket().fini();

    udp_listen_socket_.fini();
    tcp_listen_socket_.fini();

    while (io_count_) Sleep(1);

    is_running_ = false;
    io_thread_.fini();

    proactor_.fini();
}

bool AsyncEchoServer::is_running() const
{
    return is_running_;
}

bool AsyncEchoServer::StartAccept()
{
    bool succeed = false;

    for (size_t index = 0; index < kMaxListen; ++index)
    {
        ClientContext & client_context = tcp_client_context_[index];

        succeed = client_context.socket().init(AddressFamily::kInterNetwork, 
                                               SocketType::kStream, 
                                               ProtocolType::kTCP);
        if (!succeed) return false;

        succeed = client_context.socket().Associate(proactor_);
        if (!succeed) return false;

        SocketAsyncContext & arg = client_context.arg();
        arg.set_user_token(&client_context);
        arg.SetBuffer(client_context.buffer(), kMinAcceptBufferSize);
        arg.set_accept_socket(client_context.socket());
        arg.set_completion_delegate(&event_adapter_);

        succeed = tcp_listen_socket_.AcceptAsync(arg);
        if (!succeed)
            return false;
        else
            ++io_count_;
    }

    return true;
}

bool AsyncEchoServer::StartReceiveFrom()
{
    bool succeed = false;

    for (size_t index = 0; index < kMaxListen; ++index)
    {
        ClientContext & client_context = udp_client_context_[index];

        SocketAsyncContext & arg = client_context.arg();
        arg.set_user_token(&client_context);
        arg.SetBuffer(client_context.buffer(), ClientContext::kMaxUDPBufferSize);
        arg.set_remote_endpoint(client_context.remote_endpoint());
        arg.set_completion_delegate(&event_adapter_);

        succeed = udp_listen_socket_.ReceiveFromAsync(arg);
        if (!succeed)
            return false;
        else
            ++io_count_;
    }

    return true;
}

void AsyncEchoServer::IOProc()
{
    while (is_running_) proactor_.Run(1);
}

void AsyncEchoServer::OnIOCompleted(SocketAsyncContext & arg)
{
    switch (arg.last_op())
    {
    case SocketAsyncOp::kAsyncAccept:
        {
            ProcessAccept(arg);
        }
        break;
    case SocketAsyncOp::kAsyncRecv:
        {
            ProcessRecv(arg);
        }
        break;
    case SocketAsyncOp::kAsyncSend:
        {
            ProcessSend(arg);
        }
        break;
    case SocketAsyncOp::kAsyncDisconnect:
        {
            ProcessDisconnect(arg);
        }
        break;
    case SocketAsyncOp::kAsyncRecvFrom:
        {
            ProcessRecvFrom(arg);
        }
        break;
    case SocketAsyncOp::kAsyncSendTo:
        {
            ProcessSendTo(arg);
        }
        break;
    }

    --io_count_;
}

void AsyncEchoServer::ProcessAccept(SocketAsyncContext & arg)
{
    ClientContext & client_context = GetClientContext(arg);

    if (arg.error())
    {
        DisconnectClient(client_context);
        return ;
    }

    arg.SetBuffer(client_context.buffer(), ClientContext::kMaxBufferSize);
    bool succeed = client_context.socket().ReceiveAsync(arg);
    if (!succeed)
        DisconnectClient(client_context);
    else
        ++io_count_;
}

void AsyncEchoServer::ProcessRecv(SocketAsyncContext & arg)
{
    ClientContext & client_context = GetClientContext(arg);

    if (arg.error() || arg.transfered() <= 0)
    {
        DisconnectClient(client_context);
        return ;
    }

    arg.SetBuffer(client_context.buffer(), arg.transfered());
    bool succeed = client_context.socket().SendAsync(arg);
    if (!succeed)
        DisconnectClient(client_context);
    else
        ++io_count_;
}

void AsyncEchoServer::ProcessSend(SocketAsyncContext & arg)
{
    ClientContext & client_context = GetClientContext(arg);

    if (arg.error() || arg.transfered() <= 0)
    {
        DisconnectClient(client_context);
        return ;
    }

    size_t size_to_send = arg.count() - arg.transfered();
    if (size_to_send)
    {
        void * dst = reinterpret_cast<char *>(arg.buffer()) + arg.transfered();
        arg.SetBuffer(dst, size_to_send);
        bool succeed = client_context.socket().SendAsync(arg);
        if (!succeed)
            DisconnectClient(client_context);
        else
            ++io_count_;
        return ;
    }

    arg.SetBuffer(client_context.buffer(), ClientContext::kMaxBufferSize);
    bool succeed = client_context.socket().ReceiveAsync(arg);
    if (!succeed)
        DisconnectClient(client_context);
    else
        ++io_count_;
}

void AsyncEchoServer::ProcessDisconnect(SocketAsyncContext & arg)
{
    if (arg.error()) return ;

    ClientContext & client_context = GetClientContext(arg);

    arg.SetBuffer(client_context.buffer(), kMinAcceptBufferSize);
    bool succeed = client_context.socket().AcceptAsync(arg);
    if (!succeed)
        return ;
    else
        ++io_count_;
}

void AsyncEchoServer::ProcessRecvFrom(SocketAsyncContext & arg)
{
    if (arg.error() || arg.transfered() <= 0) return ;

    ClientContext & client_context = GetClientContext(arg);

    arg.SetBuffer(client_context.buffer(), arg.transfered());
    bool succeed = udp_listen_socket_.SendToAsync(arg);
    if (!succeed)
        return ;
    else
        ++io_count_;
}

void AsyncEchoServer::ProcessSendTo(SocketAsyncContext & arg)
{
    ClientContext & client_context = GetClientContext(arg);

    arg.SetBuffer(client_context.buffer(), ClientContext::kMaxUDPBufferSize);
    bool succeed = udp_listen_socket_.ReceiveFromAsync(arg);
    if (!succeed)
        return ;
    else
        ++io_count_;
}

ClientContext & AsyncEchoServer::GetClientContext(SocketAsyncContext & arg)
{
    return *reinterpret_cast<ClientContext *>(arg.user_token());
}

void AsyncEchoServer::DisconnectClient(ClientContext & client_contex)
{
    bool succeed = client_contex.socket().DisconnectAsync(client_contex.arg());
    if (!succeed)
        return ;
    else
        ++io_count_;
}

// 全异步客户端（IOCP）（TCP已完成，UDP已完成）
class AsyncClient
{
public:
    static const size_t kMaxBufferSize = 204800;    // 200KB
    static const size_t kMaxUDPBufferSize = 65536;  // 64KB

public:
    AsyncClient();
    ~AsyncClient();

    bool init(AddressFamily af, 
              SocketType type, 
              ProtocolType protocol);
    void fini();

    bool is_connected() const;
    bool is_completed() const;

    void SetSendBuffer(const void * buffer, size_t size);

    const void * send_buffer();
    size_t send_buffer_size();

    const void * recv_buffer();
    size_t recv_buffer_size();

    bool StartConnect(const IPEndPoint & remote_endpoint);
    bool StartSendTo(const IPEndPoint & remote_endpoint);

    void IOProc();

private:
    void OnIOCompleted(SocketAsyncContext & arg);
    void ProcessConnect(SocketAsyncContext & arg);
    void ProcessSend(SocketAsyncContext & arg);
    void ProcessRecv(SocketAsyncContext & arg);
    void ProcessDisconnect(SocketAsyncContext & arg);
    void ProcessSendTo(SocketAsyncContext & arg);
    void ProcessRecvFrom(SocketAsyncContext & arg);

    void Disconnect();

private:
    ClientContext tcp_client_context_;  // TCP
    ClientContext udp_client_context_;  // UDP

    Proactor proactor_;
    SocketAsyncResultAdapter<AsyncClient> event_adapter_;

    char send_buffer_[kMaxBufferSize];
    char recv_buffer_[kMaxBufferSize];
    size_t send_buffer_size_;
    size_t recv_buffer_size_;
    size_t size_sended_;

    bool is_connected_;
    bool is_completed_;
    size_t io_count_;
};

AsyncClient::AsyncClient()
    : send_buffer_size_(0)
    , recv_buffer_size_(0)
    , size_sended_(0)
    , is_connected_(false)
    , is_completed_(false)
    , io_count_(0)
{
    event_adapter_.Register(this, &AsyncClient::OnIOCompleted);
}

AsyncClient::~AsyncClient()
{
}

bool AsyncClient::init(AddressFamily af, 
                       SocketType type, 
                       ProtocolType protocol)
{
    bool succeed = false;

    Socket * socket = NULL;

    if (type == SocketType::kStream && protocol == ProtocolType::kTCP)
    {
        succeed = tcp_client_context_.socket().init(af, type, protocol);
        if(!succeed) return false;
        socket = &tcp_client_context_.socket();
    }
    else if (type == SocketType::kDgram && protocol == ProtocolType::kUDP)
    {
        succeed = udp_client_context_.socket().init(af, type, protocol);
        if(!succeed) return false;
        socket = &udp_client_context_.socket();
    }
    else
    {
        return false;
    }

    succeed = proactor_.init();
    if(!succeed) return false;

    succeed = socket->Associate(proactor_);
    if(!succeed) return false;

    return true;
}

void AsyncClient::fini()
{
    udp_client_context_.socket().fini();
    tcp_client_context_.socket().fini();

    while (io_count_) IOProc();

    is_connected_ = false;
    is_completed_ = true;

    proactor_.fini();
}

bool AsyncClient::is_connected() const
{
    return is_connected_;
}

bool AsyncClient::is_completed() const
{
    return is_completed_;
}

void AsyncClient::SetSendBuffer(const void * buffer, size_t size)
{
    memset(send_buffer_, 0, kMaxBufferSize);
    send_buffer_size_ = kMaxBufferSize < size ? kMaxBufferSize : size;
    memcpy(send_buffer_, buffer, send_buffer_size_);
}

const void * AsyncClient::send_buffer()
{
    return send_buffer_;
}

size_t AsyncClient::send_buffer_size()
{
    return send_buffer_size_;
}

const void * AsyncClient::recv_buffer()
{
    return recv_buffer_;
}

size_t AsyncClient::recv_buffer_size()
{
    return recv_buffer_size_;
}

bool AsyncClient::StartConnect(const IPEndPoint & remote_endpoint)
{
    is_completed_ = false;

    memset(recv_buffer_, 0, kMaxBufferSize);
    recv_buffer_size_ = 0;
    size_sended_ = 0;

    SocketAsyncContext & arg = tcp_client_context_.arg();
    arg.set_remote_endpoint(remote_endpoint);
    arg.set_completion_delegate(&event_adapter_);

    bool succeed = tcp_client_context_.socket().ConnectAsync(arg);
    if (!succeed)
        return false;
    else
        ++io_count_;

    return true;
}

bool AsyncClient::StartSendTo(const IPEndPoint & remote_endpoint)
{
    is_completed_ = false;

    memset(recv_buffer_, 0, kMaxBufferSize);
    recv_buffer_size_ = 0;
    size_sended_ = 0;

    SocketAsyncContext & arg = udp_client_context_.arg();
    arg.SetBuffer(send_buffer_, send_buffer_size_);
    arg.set_remote_endpoint(remote_endpoint);
    arg.set_completion_delegate(&event_adapter_);

    bool succeed = udp_client_context_.socket().SendToAsync(arg);
    if (!succeed)
        return false;
    else
        ++io_count_;

    return true;
}

void AsyncClient::IOProc()
{
    proactor_.Run(1);
}

void AsyncClient::OnIOCompleted(SocketAsyncContext & arg)
{
    switch (arg.last_op())
    {
    case SocketAsyncOp::kAsyncConnect:
        {
            ProcessConnect(arg);
        }
        break;
    case SocketAsyncOp::kAsyncSend:
        {
            ProcessSend(arg);
        }
        break;
    case SocketAsyncOp::kAsyncRecv:
        {
            ProcessRecv(arg);
        }
        break;
    case SocketAsyncOp::kAsyncDisconnect:
        {
            ProcessDisconnect(arg);
        }
        break;
    case SocketAsyncOp::kAsyncSendTo:
        {
            ProcessSendTo(arg);
        }
        break;
    case SocketAsyncOp::kAsyncRecvFrom:
        {
            ProcessRecvFrom(arg);
        }
        break;
    }

    --io_count_;
}

void AsyncClient::ProcessConnect(SocketAsyncContext & arg)
{
    if (arg.error())
    {
        Disconnect();
        return ;
    }

    is_connected_ = true;

    arg.SetBuffer(send_buffer_, send_buffer_size_);
    bool succeed = tcp_client_context_.socket().SendAsync(arg);
    if (!succeed)
        Disconnect();
    else
        ++io_count_;
}

void AsyncClient::ProcessSend(SocketAsyncContext & arg)
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
        bool succeed = tcp_client_context_.socket().SendAsync(arg);
        if (!succeed)
            Disconnect();
        else
            ++io_count_;
    }

    arg.SetBuffer(recv_buffer_ + recv_buffer_size_, arg.transfered());
    bool succeed = tcp_client_context_.socket().ReceiveAsync(arg);
    if (!succeed)
        Disconnect();
    else
        ++io_count_;
}

void AsyncClient::ProcessRecv(SocketAsyncContext & arg)
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
        bool succeed = tcp_client_context_.socket().ReceiveAsync(arg);
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

void AsyncClient::ProcessSendTo(SocketAsyncContext & arg)
{
    is_completed_ = false;

    if (arg.error() || arg.transfered() < send_buffer_size_)
    {
        is_completed_ = true;
        return ;
    }

    arg.SetBuffer(recv_buffer_, arg.transfered());
    bool succeed = udp_client_context_.socket().ReceiveFromAsync(arg);
    if (!succeed)
        return ;
    else
        ++io_count_;
}

void AsyncClient::ProcessRecvFrom(SocketAsyncContext & arg)
{
    is_completed_ = true;

    if (arg.error() || arg.transfered() < send_buffer_size_) return ;

    recv_buffer_size_ = arg.transfered();
}

void AsyncClient::ProcessDisconnect(SocketAsyncContext & arg)
{
}

void AsyncClient::Disconnect()
{
    SocketAsyncContext & arg = tcp_client_context_.arg();
    bool succeed = tcp_client_context_.socket().DisconnectAsync(arg);
    if (!succeed)
        return ;
    else
        ++io_count_;
}

// TCP读写异步客户端（IOCR）（已完成）
class AsyncHalfClient
{
public:
    static const size_t kMaxBufferSize = 204800;    // 200KB

public:
    AsyncHalfClient();
    ~AsyncHalfClient();

    bool init();
    void fini();

    bool is_completed() const;

    bool Connect(const IPEndPoint & remote_endpoint);

    void SetSendBuffer(const void * buffer, size_t size);

    const void * send_buffer();
    size_t send_buffer_size();

    const void * recv_buffer();
    size_t recv_buffer_size();

    bool StartSend();

    void IOProc();

private:
    void OnIOCompleted(SocketAsyncContext & arg);
    void ProcessSend(SocketAsyncContext & arg);
    void ProcessRecv(SocketAsyncContext & arg);

    void Disconnect();

private:
    ClientContext client_context_;

    SocketAsyncResultAdapter<AsyncHalfClient> event_adapter_;

    char send_buffer_[kMaxBufferSize];
    char recv_buffer_[kMaxBufferSize];
    size_t send_buffer_size_;
    size_t recv_buffer_size_;
    size_t size_sended_;

    bool is_completed_;
    size_t io_count_;
};

AsyncHalfClient::AsyncHalfClient()
    : send_buffer_size_(0)
    , recv_buffer_size_(0)
    , size_sended_(0)
    , is_completed_(false)
    , io_count_(0)
{
    event_adapter_.Register(this, &AsyncHalfClient::OnIOCompleted);
}

AsyncHalfClient::~AsyncHalfClient()
{
}

bool AsyncHalfClient::init()
{
    bool succeed = false;

    succeed = client_context_.socket().init(AddressFamily::kInterNetwork, 
                                            SocketType::kStream, 
                                            ProtocolType::kTCP);
    if(!succeed) return false;

    return true;
}

void AsyncHalfClient::fini()
{
    Disconnect();
    client_context_.socket().fini();

    while (io_count_) IOProc();

    is_completed_ = true;
}

bool AsyncHalfClient::is_completed() const
{
    return is_completed_;
}

bool AsyncHalfClient::Connect(const IPEndPoint & remote_endpoint)
{
    return client_context_.socket().Connect(remote_endpoint);
}

void AsyncHalfClient::SetSendBuffer(const void * buffer, size_t size)
{
    memset(send_buffer_, 0, kMaxBufferSize);
    send_buffer_size_ = kMaxBufferSize < size ? kMaxBufferSize : size;
    memcpy(send_buffer_, buffer, send_buffer_size_);
}

const void * AsyncHalfClient::send_buffer()
{
    return send_buffer_;
}

size_t AsyncHalfClient::send_buffer_size()
{
    return send_buffer_size_;
}

const void * AsyncHalfClient::recv_buffer()
{
    return recv_buffer_;
}

size_t AsyncHalfClient::recv_buffer_size()
{
    return recv_buffer_size_;
}

bool AsyncHalfClient::StartSend()
{
    is_completed_ = false;

    memset(recv_buffer_, 0, kMaxBufferSize);
    recv_buffer_size_ = 0;
    size_sended_ = 0;

    client_context_.arg().SetBuffer(send_buffer_, send_buffer_size_);
    client_context_.arg().set_completion_delegate(&event_adapter_);
    bool succeed = client_context_.socket().SendAsync(client_context_.arg());
    if (!succeed)
        return false;
    else
        ++io_count_;

    return true;
}

void AsyncHalfClient::IOProc()
{
    SleepEx(1, true);
}

void AsyncHalfClient::OnIOCompleted(SocketAsyncContext & arg)
{
    switch (arg.last_op())
    {
    case SocketAsyncOp::kAsyncSend:
        {
            ProcessSend(arg);
        }
        break;
    case SocketAsyncOp::kAsyncRecv:
        {
            ProcessRecv(arg);
        }
        break;
    }

    --io_count_;
}

void AsyncHalfClient::ProcessSend(SocketAsyncContext & arg)
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
        bool succeed = client_context_.socket().SendAsync(arg);
        if (!succeed)
            Disconnect();
        else
            ++io_count_;
    }

    arg.SetBuffer(recv_buffer_ + recv_buffer_size_, arg.transfered());
    bool succeed = client_context_.socket().ReceiveAsync(arg);
    if (!succeed)
        Disconnect();
    else
        ++io_count_;
}

void AsyncHalfClient::ProcessRecv(SocketAsyncContext & arg)
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
        bool succeed = client_context_.socket().ReceiveAsync(arg);
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

void AsyncHalfClient::Disconnect()
{
    client_context_.socket().Disconnect(false);
}

// SocketTest
class SocketTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        WORD wsaver = MAKEWORD(2, 2);
        WSADATA wsadata = {0};
        WSAStartup(wsaver, &wsadata);

        IPEndPoint endpoint(IPAddress::kIPAny, 12345);
        if (!sync_echo_server_.init(endpoint))
            return ;

        endpoint.SetPortW(23456);
        if (!async_echo_server_.init(endpoint))
            return ;

        for (size_t index = 0; index < countof(send_buffer_); ++index)
            send_buffer_[index] = GetTickCount();
    }

    static void TearDownTestCase()
    {
        sync_echo_server_.fini();
        async_echo_server_.fini();

        WSACleanup();
    }

protected:
    static SyncEchoServer sync_echo_server_;
    static AsyncEchoServer async_echo_server_;

    static int send_buffer_[51200];  // 200KB
};

SyncEchoServer SocketTest::sync_echo_server_;
AsyncEchoServer SocketTest::async_echo_server_;
int SocketTest::send_buffer_[51200];

// TCP全同步测试 SyncEchoServer SyncClient
TEST_F(SocketTest, TCPSyncServerSyncClientIO)
{
    bool succeed = false;

    // SyncEchoServer is running
    succeed = sync_echo_server_.is_running();
    ASSERT_TRUE(succeed);

    // Client init and Connect to SyncEchoServer
    Socket client;
    succeed = client.init(AddressFamily::kInterNetwork, 
                          SocketType::kStream, 
                          ProtocolType::kTCP);
    ASSERT_TRUE(succeed);

    IPEndPoint iep(IPAddress::kIPLoopback, 12345);
    succeed = client.Connect(iep);
    ASSERT_TRUE(succeed);

    // Prepare buffer
    // 1KB, 32KB, 64KB, 75KB, 200KB
    int send_buffer_size[] = {1024, 32768, 65536, 76800, 204800};

    for (size_t count = 0; count < countof(send_buffer_size); ++count)
    {
        // Send buffer to SyncEchoServer
        EXPECT_TRUE(client.CanWrite());
        const uint32_t size_to_send = send_buffer_size[count];
        uint32_t send_size = 0;
        succeed = client.Send(send_buffer_, size_to_send, send_size);
        EXPECT_TRUE(succeed);
        EXPECT_EQ(size_to_send, send_size);

        // Calculate hash of send buffer
        Hash send_buffer_hash;
        CalcHash(send_buffer_, size_to_send, send_buffer_hash);

        // Receive from SyncEchoServer
        int receive_buffer[51200] = {0};  // 200KB
        char * recv_buffer = reinterpret_cast<char *>(receive_buffer);
        uint32_t size_to_recv = 0;
        while (size_to_recv < size_to_send)
        {
            // position and remainning size to recv
            char * dst = recv_buffer + size_to_recv;
            uint32_t dst_size = size_to_send - size_to_recv;
            uint32_t recv_size = 0;
            succeed = client.Receive(dst, dst_size, recv_size);
            EXPECT_TRUE(succeed);
            if (recv_size == 0)
                break;
            size_to_recv += recv_size;
        }

        EXPECT_EQ(size_to_send, size_to_recv);

        // Calculate hash of recv buffer
        Hash recv_buffer_hash;
        CalcHash(recv_buffer, size_to_recv, recv_buffer_hash);

        EXPECT_TRUE(send_buffer_hash == recv_buffer_hash);
    }

    succeed = client.Shutdown(SocketShutdown::kBoth);
    EXPECT_TRUE(succeed);
    succeed = client.Close();
    EXPECT_TRUE(succeed);
    client.fini();
}

// UDP全同步测试 SyncEchoServer SyncClient
TEST_F(SocketTest, UDPSyncServerSyncClientIO)
{
    bool succeed = false;

    // SyncEchoServer is running
    succeed = sync_echo_server_.is_running();
    ASSERT_TRUE(succeed);

    // Client init
    Socket client;
    succeed = client.init(AddressFamily::kInterNetwork, 
                          SocketType::kDgram, 
                          ProtocolType::kUDP);
    ASSERT_TRUE(succeed);

    IPEndPoint iep(IPAddress::kIPLoopback, 12345);

    // Prepare buffer
    // 64KB-1B-20B-8B=65507B(max)
    const uint32_t size_to_send = 65508;
    uint32_t send_size = 0;
    succeed = client.SendTo(send_buffer_, size_to_send, iep, send_size);
    EXPECT_FALSE(succeed);
    EXPECT_EQ(0, send_size);

    // 1KB, 32KB, 64KB-1B-20B-8B=65507B(max)
    int send_buffer_size[] = {1, 64, 1024, 32768, 65507};

    for (size_t count = 0; count < countof(send_buffer_size); ++count)
    {
        // Send buffer to SyncEchoServer
        ASSERT_TRUE(client.CanWrite());
        const uint32_t size_to_send = send_buffer_size[count];
        uint32_t send_size = 0;
        succeed = client.SendTo(send_buffer_, size_to_send, iep, send_size);
        EXPECT_TRUE(succeed);
        EXPECT_EQ(size_to_send, send_size);

        // Calculate hash of send buffer
        Hash send_buffer_hash;
        CalcHash(send_buffer_, send_size, send_buffer_hash);

        // Receive from SyncEchoServer
        int receive_buffer[16384] = {0};  // 64KB
        char * recv_buffer = reinterpret_cast<char *>(receive_buffer);
        const uint32_t size_to_recv = send_size;
        uint32_t recv_size = 0;
        succeed = client.ReceiveFrom(recv_buffer, size_to_recv, recv_size, iep);
        EXPECT_TRUE(succeed);
        EXPECT_EQ(size_to_recv, recv_size);

        // Calculate hash of recv buffer
        Hash recv_buffer_hash;
        CalcHash(recv_buffer, recv_size, recv_buffer_hash);

        EXPECT_TRUE(send_buffer_hash == recv_buffer_hash);
    }

    client.fini();
}

// TCP全异步测试 AsyncEchoServer AsyncClient
TEST_F(SocketTest, TCPAsyncServerAsyncClientIO)
{
    bool succeed = false;

    // AsyncEchoServer is running
    succeed = async_echo_server_.is_running();
    ASSERT_TRUE(succeed);

    // Prepare buffer
    // 1KB, 32KB, 64KB, 75KB, 200KB
    int send_buffer_size[] = {1024, 32768, 65536, 76800, 204800};

    for (size_t count = 0; count < countof(send_buffer_size); ++count)
    {
        AsyncClient async_client;

        // AsyncClient init
        succeed = async_client.init(AddressFamily::kInterNetwork, 
                                    SocketType::kStream, 
                                    ProtocolType::kTCP);
        EXPECT_TRUE(succeed);

        // AsyncClient set send buffer
        async_client.SetSendBuffer(send_buffer_, send_buffer_size[count]);

        // AsyncClient start connect
        IPEndPoint remote_endpoint(IPAddress::kIPLoopback, 23456);
        succeed = async_client.StartConnect(remote_endpoint);
        EXPECT_TRUE(succeed);

        while (!async_client.is_completed()) async_client.IOProc();

        // AsyncClient is running
        succeed = async_client.is_connected();
        EXPECT_TRUE(succeed);

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

        async_client.fini();
    }
}

// UDP全异步测试 AsyncEchoServer AsyncClient
TEST_F(SocketTest, UDPAsyncServerAsyncClientIO)
{
    bool succeed = false;

    // AsyncEchoServer is running
    succeed = async_echo_server_.is_running();
    ASSERT_TRUE(succeed);

    // Prepare buffer
    // 1KB, 32KB, 64KB-1B-20B-8B=65507B(max)
    int send_buffer_size[] = {1, 64, 1024, 32768, 65507, 65508, 65536, 76800};

    for (size_t count = 0; count < countof(send_buffer_size); ++count)
    {
        AsyncClient async_client;

        // AsyncClient init
        succeed = async_client.init(AddressFamily::kInterNetwork, 
                                    SocketType::kDgram, 
                                    ProtocolType::kUDP);
        EXPECT_TRUE(succeed);

        // AsyncClient set send buffer
        async_client.SetSendBuffer(send_buffer_, send_buffer_size[count]);

        // AsyncClient start send to
        IPEndPoint remote_endpoint(IPAddress::kIPLoopback, 23456);
        succeed = async_client.StartSendTo(remote_endpoint);
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

        if (send_buffer_size[count] > 65507)
            EXPECT_FALSE(send_buffer_hash == recv_buffer_hash);
        else
            EXPECT_TRUE(send_buffer_hash == recv_buffer_hash);

        async_client.fini();
    }
}

// TCP读写异步测试 AsyncEchoServer AsyncHalfClient
TEST_F(SocketTest, TCPAsyncServerAsyncHalfClientIO)
{
    bool succeed = false;

    // AsyncEchoServer is running
    succeed = async_echo_server_.is_running();
    ASSERT_TRUE(succeed);

    AsyncHalfClient async_client;

    // AsyncClient init
    succeed = async_client.init();
    EXPECT_TRUE(succeed);

    // AsyncClient start connect
    IPEndPoint remote_endpoint(IPAddress::kIPLoopback, 23456);
    succeed = async_client.Connect(remote_endpoint);
    EXPECT_TRUE(succeed);

    // Prepare buffer
    // 1KB, 32KB, 64KB, 75KB, 200KB
    int send_buffer_size[] = {1024, 32768, 65536, 76800, 204800};

    for (size_t count = 0; count < countof(send_buffer_size); ++count)
    {
        // AsyncClient set send buffer
        async_client.SetSendBuffer(send_buffer_, send_buffer_size[count]);

        succeed = async_client.StartSend();
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
}
