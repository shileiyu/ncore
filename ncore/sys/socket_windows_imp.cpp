#include <ncore/base/buffer.h>
#include "named_event.h"
#include "proactor.h"
#include "socket_async_event_args.h"
#include "socket.h"

namespace ncore
{

typedef BOOL (__stdcall * AcceptEx_t)(SOCKET sListenSocket, 
                                      SOCKET sAcceptSocket,
                                      PVOID lpOutputBuffer, 
                                      DWORD dwReceiveDataLength,
                                      DWORD dwLocalAddressLength,
                                      DWORD dwRemoteAddressLength,
                                      LPDWORD lpdwBytesReceived,
                                      LPOVERLAPPED lpOverlapped);

typedef BOOL (_stdcall * ConnectEx_t)(SOCKET s,
                                      const struct sockaddr *name,
                                      int namelen,
                                      PVOID lpSendBuffer,
                                      DWORD dwSendDataLength,
                                      LPDWORD lpdwBytesSent,
                                      LPOVERLAPPED lpOverlapped);

typedef BOOL (__stdcall * DisconnectEx_t)(SOCKET hSocket,
                                          LPOVERLAPPED lpOverlapped,
                                          DWORD dwFlags,
                                          DWORD reserved);

static const size_t kAddressBufferSize  = sizeof(sockaddr_in) + 16;
static const size_t kMinAcceptBufferSize = kAddressBufferSize * 2;


class SocketRoutines
{
private:
    friend class Socket;

    static void __stdcall OnCompleted(DWORD dwError,
                                      DWORD cbTransferred,
                                      LPWSAOVERLAPPED lpOverlapped,
                                      DWORD dwFlags)
    {
        auto & sae = *reinterpret_cast<SocketAsyncContext*>(lpOverlapped);
        sae.socket_flags_ = dwFlags;
        sae.OnCompleted(dwError, cbTransferred);
    }

    static AcceptEx_t GetAcceptExAddress(SOCKET socket)
    {
        static AcceptEx_t AcceptEx = 0;
        static GUID GuidAcceptEx = WSAID_ACCEPTEX;

        if(AcceptEx == 0)
        {
            DWORD byte_received = 0;
            WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, 
                &GuidAcceptEx, sizeof(GuidAcceptEx), 
                &AcceptEx, sizeof(AcceptEx), 
                &byte_received, 0, 0);
        }
        return AcceptEx;
    }

    static ConnectEx_t GetConnectExAddress(SOCKET socket)
    {
        static ConnectEx_t ConnectEx = 0;
        static GUID GuidConnectEx = WSAID_CONNECTEX;

        if(ConnectEx == 0)
        {
            DWORD byte_received = 0;
            WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, 
                &GuidConnectEx, sizeof(GuidConnectEx), 
                &ConnectEx, sizeof(ConnectEx), 
                &byte_received, 0, 0);
        }
        return ConnectEx;
    }

    static DisconnectEx_t GetDisconnectExAddress(SOCKET socket)
    {
        static DisconnectEx_t DisconnectEx = 0;
        static GUID GuidDisconnectEx = WSAID_DISCONNECTEX;

        if(DisconnectEx == 0)
        {
            DWORD byte_received = 0;
            WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, 
                &GuidDisconnectEx, sizeof(GuidDisconnectEx), 
                &DisconnectEx, sizeof(DisconnectEx), 
                &byte_received, 0, 0);
        }
        return DisconnectEx;
    }

    static bool GetProtocolInfo(SOCKET socket, 
                                AddressFamily & af,
                                SocketType & type,
                                ProtocolType & protocol)
    {
        WSAPROTOCOL_INFO pi = {0};
        char * ppi = reinterpret_cast<char *>(&pi);
        int pi_size = sizeof(pi);
        if(getsockopt(socket, SOL_SOCKET, SO_PROTOCOL_INFO, ppi, &pi_size))
        {
            return false;
        }

        af = static_cast<AddressFamily>(pi.iAddressFamily);
        type = static_cast<SocketType>(pi.iSocketType);
        protocol = static_cast<ProtocolType>(pi.iProtocol);
        return true;
    }   
};


Socket::Socket()
    : s_(INVALID_SOCKET),
      io_handler_(0)
{

}

Socket::~Socket()
{
    Close();
}

Socket::Socket(Socket && obj)
    : s_(INVALID_SOCKET),
      io_handler_(0)
{
    std::swap(s_, obj.s_);
    std::swap(io_handler_, obj.io_handler_);
}

Socket & Socket::operator = (Socket && obj)
{
    std::swap(s_, obj.s_);
    std::swap(io_handler_, obj.io_handler_);
    return *this;
}

bool Socket::init(AddressFamily af,
                  SocketType type,
                  ProtocolType protocol)
{
    if(s_ != INVALID_SOCKET)
        return true;

    s_= WSASocket(af, type, protocol, 0, 0, WSA_FLAG_OVERLAPPED);

    return s_ != INVALID_SOCKET ? true : false;
}

void Socket::fini()
{
    Close();
    return;
}

bool Socket::Close()
{
    auto ss = s_;
    if(ss == INVALID_SOCKET)
        return false;
    s_ = INVALID_SOCKET;
    if(closesocket(ss))
        return false;
    return true;
}

bool Socket::Bind(const IPEndPoint & endpoint)
{
    auto sa_ptr = reinterpret_cast<const sockaddr *>(&endpoint.ep_);
    if(!bind(s_, sa_ptr, endpoint.ep_size_))
        return true;
    return false;
}

bool Socket::Listen(int backlog)
{
    return listen(s_, backlog) ? false : true;
}

bool Socket::Shutdown(SocketShutdown how)
{
    return shutdown(s_, how) ? false : true;
}

Socket Socket::Accept()
{
    return Accept(-1);
}

Socket Socket::Accept(uint32_t timeout)
{
    Socket invalid_socket;
    Socket accept_socket;

    if(s_ == INVALID_SOCKET)
        return invalid_socket;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return invalid_socket;

    SocketAsyncContext args(complete_event);
    args.SuppressIOCP();

    AddressFamily af;
    SocketType type;
    ProtocolType protocol;

    if(!SocketRoutines::GetProtocolInfo(s_, af, type, protocol))
        return invalid_socket;

    if(!accept_socket.init(af, type, protocol))
        return invalid_socket;

    args.set_accept_socket(accept_socket);

    FixedBuffer<kMinAcceptBufferSize> output;
    args.SetBuffer(output.data(), output.capacity());

    if(!AcceptAsync(args))
        return invalid_socket;

    if(!complete_event.Wait(timeout))
        Cancel();

    if(!WaitSocketAsyncEvent(args))
        return invalid_socket;

    return std::move(accept_socket);
}

bool Socket::AcceptAsync(SocketAsyncContext & args)
{
    AcceptEx_t AcceptEx = SocketRoutines::GetAcceptExAddress(s_);

    if(s_ == INVALID_SOCKET)
        return false;

    if(AcceptEx == 0)
        return false;

    if(args.count_ < kMinAcceptBufferSize) 
        return false;

    if(args.accept_socket_ == 0)
        return false;

    if(args.accept_socket_->IsValid() == false)
        return false;

    if(args.buffer() == 0)
        return false;

    size_t output_size = args.count() - kMinAcceptBufferSize;
    auto byte_transed_ptr = reinterpret_cast<DWORD *>(&args.transfered_);
    args.last_op_ = SocketAsyncOp::kAsyncAccept;
    if(!AcceptEx(s_, args.accept_socket_->s_, args.buffer(), output_size,
                 kAddressBufferSize, kAddressBufferSize, 
                 byte_transed_ptr, &args.overlapped_))
    {
        DWORD last_err = WSAGetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    return true;
}

bool Socket::Connect(const IPEndPoint & endpoint)
{
    return Connect(endpoint, -1);
}

bool Socket::Connect(const IPEndPoint & endpoint, uint32_t timeout)
{
    if(s_ == INVALID_SOCKET)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    SocketAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.set_remote_endpoint(endpoint);

    if(!ConnectAsync(args))
        return false;

    if(!complete_event.Wait(timeout))
        Cancel();

    if(!WaitSocketAsyncEvent(args))
        return false;

    return true;
}

bool Socket::ConnectAsync(SocketAsyncContext & args)
{
    ConnectEx_t ConnectEx = SocketRoutines::GetConnectExAddress(s_);

    if(s_ == INVALID_SOCKET)
        return false;

    if(ConnectEx == 0)
        return false;

    if(!Bind(IPEndPoint::kAny))
        if(WSAGetLastError() != WSAEINVAL)
            return false;			//绑定失败

    IPEndPoint & ep = args.remote_endpoint();
    auto sa_ptr = reinterpret_cast<sockaddr *>(&ep.ep_);
    auto byte_transed_ptr = reinterpret_cast<DWORD *>(&args.transfered_);
    args.connect_socket_ = this;
    args.last_op_ = SocketAsyncOp::kAsyncConnect;
    if(!ConnectEx(s_, sa_ptr, ep.ep_size_, args.buffer(), args.count(),
                  byte_transed_ptr, &args.overlapped_))
    {
        DWORD last_err = WSAGetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    return true;
}

void Socket::Disconnect(bool reuse)
{
    if(s_ == INVALID_SOCKET)
        return;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return;

    SocketAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.set_reuse(reuse);
    
    if(!DisconnectAsync(args))
        return;

    if(!complete_event.Wait(-1))
        return;

    if(!WaitSocketAsyncEvent(args))
        return;

    return;
}

bool Socket::DisconnectAsync(SocketAsyncContext & args)
{
    DisconnectEx_t DisconnectEx = SocketRoutines::GetDisconnectExAddress(s_);

    if(s_ == INVALID_SOCKET)
        return false;
    if(DisconnectEx == 0)
        return false;

    args.last_op_ = SocketAsyncOp::kAsyncDisconnect;
    if(!DisconnectEx(s_, &args.overlapped_, 
                     args.reuse()? TF_REUSE_SOCKET: 0, 0))
    {
        DWORD last_err = WSAGetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }

    return true;
}

bool Socket::Receive(void * data, uint32_t size_to_recv, uint32_t & transfered)
{
    return Receive(data, size_to_recv, -1, transfered);
}

bool Socket::Receive(void * data, uint32_t size_to_recv, uint32_t timeout,
                     uint32_t & transfered)
{
    if(s_ == INVALID_SOCKET)
        return false;

    if(data == 0)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    SocketAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.SetBuffer(data, size_to_recv);
    if(!ReceiveAsync(args))
        return false;
    
    if(!complete_event.Wait(timeout))
        Cancel();

    if(!WaitSocketAsyncEvent(args))
        return false;

    transfered = args.transfered();
    return true;
}

bool Socket::ReceiveAsync(SocketAsyncContext & args)
{
    if(s_ == INVALID_SOCKET)
        return false;

    char * buffer = reinterpret_cast<char *>(args.buffer());
    if(buffer == 0)
        return false;

    WSABUF wsa_buf = {args.count(), buffer};
    DWORD * byte_transed_ptr = 0;//reinterpret_cast<DWORD *>(&args.transfered_);
    auto socket_flag_ptr = reinterpret_cast<DWORD *>(&args.socket_flags_);
    auto iocr = SocketRoutines::OnCompleted;
    if(args.completion_delegate_ == 0 || io_handler_ != 0)
        iocr = 0;
    args.last_op_ = SocketAsyncOp::kAsyncRecv;
    if(WSARecv(s_, &wsa_buf, 1, byte_transed_ptr, socket_flag_ptr, 
               &args.overlapped_, iocr))
    {
        DWORD last_err = WSAGetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    return true;
}

bool Socket::Send(const void * data, uint32_t size_to_send, 
                  uint32_t & transfered)
{
    return Send(data, size_to_send, -1, transfered);
}

bool Socket::Send(const void * data, uint32_t size_to_send, uint32_t timeout,
                  uint32_t & transfered)
{
    if(s_ == INVALID_SOCKET)
        return false;

    if(data == 0)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    SocketAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.SetBuffer(data, size_to_send);
    if(!SendAsync(args))
        return false;

    if(!complete_event.Wait(timeout))
        Cancel();

    if(!WaitSocketAsyncEvent(args))
        return false;

    transfered = args.transfered();
    return true;
}

bool Socket::SendAsync(SocketAsyncContext & args)
{
    if(s_ == INVALID_SOCKET)
        return false;

    char * buffer = reinterpret_cast<char *>(args.buffer());
    if(buffer == 0)
        return false;

    WSABUF wsa_buf = {args.count(), buffer};
    DWORD * byte_transed_ptr = 0;//reinterpret_cast<DWORD *>(&args.transfered_);
    auto iocr = SocketRoutines::OnCompleted;
    if(args.completion_delegate_ == 0 || io_handler_ != 0)
        iocr = 0;
    args.last_op_ = SocketAsyncOp::kAsyncSend;
    if(WSASend(s_, &wsa_buf, 1, byte_transed_ptr, args.socket_flags_, 
               &args.overlapped_, iocr))
    {
        DWORD last_err = WSAGetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    return true;
}

bool Socket::ReceiveFrom(void * data, uint32_t size_to_recv, 
                        uint32_t & transfered, IPEndPoint & endpoint)
{
    return ReceiveFrom(data, size_to_recv, -1, transfered, endpoint);
}

bool Socket::ReceiveFrom(void * data, uint32_t size_to_recv, uint32_t timeout,
                        uint32_t & transfered, IPEndPoint & endpoint)
{
    if(s_ == INVALID_SOCKET)
        return false;

    if(data == 0)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    SocketAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.SetBuffer(data, size_to_recv);
    if(!ReceiveFromAsync(args))
        return false;

    if(!complete_event.Wait(timeout))
        Cancel();

    if(!WaitSocketAsyncEvent(args))
        return false;

    endpoint = args.remote_endpoint();
    transfered = args.transfered();
    return true;
}

bool Socket::ReceiveFromAsync(SocketAsyncContext & args)
{
    if(s_ == INVALID_SOCKET)
        return false;

    char * buffer = reinterpret_cast<char *>(args.buffer());
    if(buffer == 0)
        return false;

    WSABUF wsa_buf = {args.count(), buffer};
    DWORD * byte_transed_ptr = 0;//reinterpret_cast<DWORD *>(&args.transfered_);
    auto socket_flag_ptr = reinterpret_cast<DWORD *>(&args.socket_flags_);
    auto sa_ptr = reinterpret_cast<sockaddr *>(&args.remote_endpoint_.ep_);
    auto sa_size_ptr = reinterpret_cast<int *>(&args.remote_endpoint_.ep_size_);
    auto iocr = SocketRoutines::OnCompleted;
    if(args.completion_delegate_ == 0 || io_handler_ != 0)
        iocr = 0;
    args.last_op_ = SocketAsyncOp::kAsyncRecvFrom;
    if(WSARecvFrom(s_, &wsa_buf, 1, byte_transed_ptr, socket_flag_ptr, 
                   sa_ptr, sa_size_ptr, &args.overlapped_, iocr))
    {
        DWORD last_err = WSAGetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    return true;
}

bool Socket::SendTo(const void * data, uint32_t size_to_send, 
                    const IPEndPoint & endpoint, uint32_t & transfered)
{
    return SendTo(data, size_to_send, endpoint, -1, transfered);
}

bool Socket::SendTo(const void * data, uint32_t size_to_send, 
                    const IPEndPoint & endpoint, uint32_t timeout,
                    uint32_t & transfered)
{
    if(s_ == INVALID_SOCKET)
        return false;

    if(data == 0)
        return false;

    NamedEvent complete_event;
    if(!complete_event.init(true, false))
        return false;

    SocketAsyncContext args(complete_event);
    args.SuppressIOCP();
    args.SetBuffer(data, size_to_send);
    args.set_remote_endpoint(endpoint);

    if(!SendToAsync(args))
        return false;

    if(!complete_event.Wait(timeout))
        Cancel();

    if(!WaitSocketAsyncEvent(args))
        return false;

    transfered = args.transfered();

    return true;
}

bool Socket::SendToAsync(SocketAsyncContext & args)
{
    if(s_ == INVALID_SOCKET)
        return false;

    char * buffer = reinterpret_cast<char *>(args.buffer());
    if(buffer == 0)
        return false;

    WSABUF wsa_buf = {args.count(), buffer};
    DWORD * byte_transed_ptr = 0;//reinterpret_cast<DWORD *>(&args.transfered_);
    auto sa_ptr = reinterpret_cast<sockaddr *>(&args.remote_endpoint_.ep_);
    auto iocr = SocketRoutines::OnCompleted;
    if(args.completion_delegate_ == 0 || io_handler_ != 0)
        iocr = 0;
    args.last_op_ = SocketAsyncOp::kAsyncSendTo;
    if(WSASendTo(s_, &wsa_buf, 1, byte_transed_ptr, args.socket_flags_, 
                  sa_ptr, args.remote_endpoint_.ep_size_, &args.overlapped_, 
                  iocr))
    {
        DWORD last_err = WSAGetLastError();
        if(last_err != ERROR_IO_PENDING)
            return false;
    }
    return true;
}

bool Socket::CanRead()
{
    if(s_ == INVALID_SOCKET)
        return false;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    timeval timeout = {0, 0};
    FD_SET(s_, &read_fds);
    int result = select(1, &read_fds, NULL, NULL, &timeout);
    if (result <= 0)
        return false;

    return FD_ISSET(s_, &read_fds) != 0;
}

bool Socket::CanWrite()
{
    if(s_ == INVALID_SOCKET)
        return false;

    fd_set write_fds;
    FD_ZERO(&write_fds);
    timeval timeout = {0, 0};
    FD_SET(s_, &write_fds);
    int result = select(1, NULL, &write_fds, NULL, &timeout);
    if (result <= 0)
        return false;

    return FD_ISSET(s_, &write_fds) != 0;
}

bool Socket::IsValid()
{
    return s_ != INVALID_SOCKET;
}

bool Socket::Cancel()
{
    if(s_ == INVALID_SOCKET)
        return false;

    return ::CancelIo((HANDLE)s_) != FALSE;
}

bool Socket::Associate(Proactor & io)
{
    if(s_ == INVALID_SOCKET)
        return false;

    if(io.Associate(*this))
    {
        io_handler_ = &io;
        return true;
    }
    return false;
}

void * Socket::GetPlatformHandle()
{
    return reinterpret_cast<void *>(s_);
}

void Socket::OnCompleted(AsyncContext & args,
                         uint32_t error, 
                         uint32_t transfered)
{
    auto & sock_args = static_cast<SocketAsyncContext&>(args);
    switch(sock_args.last_op())
    {
    case SocketAsyncOp::kAsyncAccept:
        {
            setsockopt(sock_args.accept_socket_->s_, SOL_SOCKET, 
                       SO_UPDATE_ACCEPT_CONTEXT, (char *)&s_, sizeof(s_));
        }
        break;
    case SocketAsyncOp::kAsyncConnect:
        {
            setsockopt(sock_args.connect_socket_->s_, SOL_SOCKET, 
                       SO_UPDATE_CONNECT_CONTEXT, 0, 0);
        }
        break;  
    }
    sock_args.OnCompleted(error, transfered);
}

bool Socket::WaitSocketAsyncEvent(SocketAsyncContext & args)
{
    bool succeed = true;
    DWORD error = 0;
    DWORD flag = 0;
    DWORD transed = 0;

    if(!WSAGetOverlappedResult(s_, &args.overlapped_, &transed, TRUE, &flag))
    {
        error = WSAGetLastError();
        succeed = false;
    }
    args.socket_flags_ = flag;
    OnCompleted(args, error, transed);
    return succeed;
}


}