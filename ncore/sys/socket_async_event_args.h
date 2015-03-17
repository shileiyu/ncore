#ifndef NCORE_SYS_SOCKET_ASYNC_EVENT_ARGS_H_
#define NCORE_SYS_SOCKET_ASYNC_EVENT_ARGS_H_

#include <ncore/ncore.h>
#include <ncore/utils/async_result_delegate.h>
#include <ncore/utils/async_result_adapter.h>
#include "async_context.h"
#include "ip_endpoint.h"

namespace ncore
{


class Socket;

/*Socket异步操作类型*/
namespace SocketAsyncOp
{
enum Value
{
    kAsyncUnknow = 0,
    kAsyncAccept,
    kAsyncConnect,
    kAsyncDisconnect,
    kAsyncSend,
    kAsyncRecv,
    kAsyncSendTo,
    kAsyncRecvFrom,
};
}

class SocketAsyncContext;

typedef AsyncResultDelegate<SocketAsyncContext> SocketAsyncResultDelegate;
typedef AsyncResultHandler<SocketAsyncContext>  SocketAsyncResultHandler;

class SocketAsyncContext : public AsyncContext
{
public:
    SocketAsyncContext();
    SocketAsyncContext(NamedEvent & e);

    void SetBuffer(const void * buffer, size_t size);
    void SetBuffer(void * buffer, size_t size);

    void * buffer() const;
    size_t count() const;
    uint32_t error() const;
    uint32_t transfered() const;
    uint32_t socket_flags() const;
    SocketAsyncOp::Value last_op() const;
    Socket * accept_socket() const;
    Socket * connect_socket() const;
    IPEndPoint remote_endpoint() const;
    bool reuse() const;

    void set_accept_socket(Socket & socket);
    void set_remote_endpoint(const IPEndPoint & ep);
    void set_reuse(bool value);
    void set_completion_delegate(SocketAsyncResultHandler * handler);
private:
    void OnCompleted(uint32_t error, uint32_t transfered);
private:
    void * buffer_;
    size_t count_;
    uint32_t error_;
    uint32_t transfered_;
    uint32_t socket_flags_;
    Socket * accept_socket_;
    Socket * connect_socket_;
    SocketAsyncOp::Value last_op_;
    IPEndPoint remote_endpoint_;
    SocketAsyncResultDelegate completion_delegate_;
    bool reuse_;

    friend class Socket;
    friend class SocketRoutines;
};

template <typename Adaptee>
using SocketAsyncResultAdapter = 
AsyncResultAdapter<Adaptee, SocketAsyncContext>;


}

#endif