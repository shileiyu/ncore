#include "socket_async_event_args.h"


namespace ncore
{


SocketAsyncContext::SocketAsyncContext()
    : AsyncContext(), error_(0), transfered_(0),
      buffer_(0), count_(0), socket_flags_(0),
      accept_socket_(0), connect_socket_(0), 
      last_op_(SocketAsyncOp::kAsyncUnknow),
      completion_delegate_(), reuse_(false)
{
}

SocketAsyncContext::SocketAsyncContext(NamedEvent & e)
    : AsyncContext(e), error_(0), transfered_(0),
      buffer_(0), count_(0), socket_flags_(0),
      accept_socket_(0), connect_socket_(0), 
      last_op_(SocketAsyncOp::kAsyncUnknow),
      completion_delegate_(), reuse_(false)
{
}

void SocketAsyncContext::SetBuffer(const void * buffer, size_t size)
{
    buffer_ = const_cast<void *>(buffer);
    count_ = size;
}

void SocketAsyncContext::SetBuffer(void * buffer, size_t size)
{
    buffer_ = buffer;
    count_ = size;
}

void * SocketAsyncContext::buffer() const
{
    return buffer_;
}

size_t SocketAsyncContext::count() const
{
    return count_;
}

uint32_t SocketAsyncContext::error() const
{
    return error_;
}

uint32_t SocketAsyncContext::transfered() const
{
    return transfered_;
}

uint32_t SocketAsyncContext::socket_flags() const
{
    return socket_flags_;
}

SocketAsyncOp::Value SocketAsyncContext::last_op() const
{
    return last_op_;
}

IPEndPoint SocketAsyncContext::remote_endpoint() const
{
    return remote_endpoint_;
}

Socket * SocketAsyncContext::accept_socket() const
{
    return accept_socket_;
}

Socket * SocketAsyncContext::connect_socket() const
{
    return connect_socket_;
}

bool SocketAsyncContext::reuse() const
{
    return reuse_;
}

void SocketAsyncContext::set_accept_socket(Socket & socket)
{
    accept_socket_ = &socket;
}

void SocketAsyncContext::set_remote_endpoint(const IPEndPoint & ep)
{
    remote_endpoint_ = ep;
}

void SocketAsyncContext::set_reuse(bool value)
{
    reuse_ = value;
}

void SocketAsyncContext::set_completion_delegate(
    SocketAsyncResultHandler * handler
) {
    completion_delegate_ = handler;
}

void SocketAsyncContext::OnCompleted(uint32_t error, uint32_t transfered)
{
    error_ = error;
    transfered_ = transfered;
    completion_delegate_(*this);
}

}