#ifndef NCORE_SYS_SOCKET_H_
#define NCORE_SYS_SOCKET_H_

#include <ncore/ncore.h>
#include <ncore/base/object.h>
#include "io_portal.h"
#include "ip_endpoint.h"
#include "network_define.h"

/*!
@file socket.h
*/
namespace ncore
{


class Proactor;
class SocketAsyncContext;

/*! 套接字类\n
可以创建两种方式的套接字：\n
一种是TCP，即面向连接的可靠的传输协议；\n
另一种是UDP，即无连接的，不可靠的数据报传输协议。\n
通信模式分为“同步（阻塞）”模式和“异步（非阻塞）”模式。\n
*/
class Socket : public NonCopyableObject,
               public IOPortal
{
public:
    Socket();
    ~Socket();

    Socket(Socket && obj);
    Socket & operator = (Socket && obj);

    /*! 初始化，创建套接字
    @param[in] af         地址族描述。
    @param[in] type       套接字的类型描述。
    @param[in] protocol   协议类型。
    @return 初始化成功后返回true；否则返回false。
    @remark 如果af为kInterNetworkV6，会创建双模式的套接字，该套接字能同时使用IPv4和IPv6。\n
    */
    bool init(AddressFamily af, 
              SocketType type,
              ProtocolType protocol);

    /*! 反初始化，销毁套接字
    @remark 等同于Close方法。
    */
    void fini();

    /*! 销毁套接字，释放资源。
    @return 销毁成功后返回true；否则返回false。
    @remark 套接字销毁后将不再可用，除非重新init。\n
            如果该套接字已经建立了连接，则在Close之前，应该使用Shutdown，以确保数据完全接收和发送。\n
    */
    bool Close();

    /*! 绑定
    @param[in] endpoint 绑定到指定的IP地址和Port。
    @return 绑定成功后返回true；否则返回false。
    @remark 做为服务端，都应该使用此方法绑定到IPEndPoint上，然后才能进行监听、接受连接和数据的收发。\n
    */
    bool Bind(const IPEndPoint & endpoint);

    /*! 监听
    @param[in] backlog 最大等待数量，默认为25。
    @return 监听成功后返回true；否则返回false。
    @remark 做为TCP服务端，应该使用此方法来进入监听模式，然后才能接受客户端的连接。\n
            在此之前，需要先使用Bind方法。\n
    */
    bool Listen(int backlog = 25);

    /*! 关闭连接
    @param[in] how 关闭连接的方式。
    @return 关闭成功后返回true；否则返回false。
    @remark 做为TCP使用，在销毁之前应该使用此方法，以确保数据能都正确接收和发送。\n
            当使用此方法后，意味着即将关闭套接字，接着应该使用Close方法。\n
    */
    bool Shutdown(SocketShutdown how);

    /*! 同步（阻塞）接受连接
    @return 接受到连接后返回一个新的Socket。
    @remark 做为TCP服务端，使用此方法来接受客户端的连接，接受成功后，可以用返回的Socket与客户端进行通信。\n
            在此之前，需要先使用Listen方法。\n
    */
    Socket Accept();

    /*! 同步（阻塞）接受连接
    @param[in] timeout 接受的超时时间。
    @return 在timeout时间内返回接受到的Socket，需要使用IsValid判断此Socket是否有效。
    */
    Socket Accept(uint32_t timeout);

    /*! 异步（非阻塞）接受连接
    @param[in] args 异步操作的上下文对象。
    @return 发起异步接受成功后返回true；否则，返回false。
    @remark 做为TCP服务端，使用此方法来发起异步接受客户端的连接。\n
            在此之前，需要将当前套接字与一个Proactor进行关联。\n
            上下文对象需要设置用于接受客户端连接的Socket，该Socket需要与Proactor进行关联，
            当连接成功后，服务端可以使用该Socket与客户端进行通信。
            同时，上下文对象还需要设置一个用于数据收发的缓冲区，指定缓冲区的最小大小为kMinAcceptBufferSize，
            如果大于kMinAcceptBufferSize，意味着接受成功的同时会接收到大于kMinAcceptBufferSize的部分的数据。
            另外，上下文对象还需要设置一个异步接受完成的回调函数，回调函数可以通过SocketAsyncEventAdapter进行适配，
            当异步接受完成时，在此回调函数中会返回该上下文对象。\n
    */
    bool AcceptAsync(SocketAsyncContext & args);

    /*! 同步（阻塞）连接
    @param[in] endpoint 连接到指定的IP地址和Port。
    @return 连接成功后返回true；否则返回false。
    @remark 做为TCP服务端，使用此方法来连接到服务端。\n
    */
    bool Connect(const IPEndPoint & endpoint);

    /*! 同步（阻塞）连接
    @param[in] endpoint 连接到指定的IP地址和Port。
    @param[in] timeout  连接的超时时间。
    @return 在timeout时间内进行连接。如果连接成功，返回true；否则返回false。
    */
    bool Connect(const IPEndPoint & endpoint, uint32_t timeout);

    /*! 异步（非阻塞）连接
    @param[in] args 异步操作的上下文对象。
    @return 发起异步连接成功后返回true；否则，返回false。
    @remark 做为TCP客户端，使用此方法来发起异步连接到服务端。\n
            在此之前，需要将当前套接字与一个Proactor进行关联。\n
            上下文对象需要设置服务端的IPEndPoint。
            同时，上下文对象还需要设置一个异步连接完成的回调函数，回调函数可以通过SocketAsyncEventAdapter进行适配，
            当异步连接完成时，在此回调函数中会返回该上下文对象。\n
    */
    bool ConnectAsync(SocketAsyncContext & args);

    /*! 同步（阻塞）断开连接
    @param[in] reuse 断开后是否重用。
    @remark 做为TCP使用，用来断开套接字的连接。\n
            如果reuse为false，则该套接字将不再可用；否则该套接字还能继续使用。\n
    */
    void Disconnect(bool reuse);

    /*! 异步（非阻塞）断开连接
    @param[in] args 异步操作的上下文对象。
    @return 发起异步断开连接成功后返回true；否则，返回false。
    @remark 做为TCP使用，使用此方法来发起异步断开连接。\n
            发起异步断开连接的Socket应该是异步接受或者异步连接的Socket。\n
    */
    bool DisconnectAsync(SocketAsyncContext & args);

    /*! 同步（阻塞）接收数据
    @param[in] data           接收数据的缓冲区。
    @param[in] size_to_recv   期望接收的数据的大小。
    @param[out] transfered    当前接收的数据的大小。
    @return 接收成功后返回true；否则返回false。
    @remark 做为TCP使用，用来接收数据。\n
            如果发送方断开连接或者关闭，则transfered等于0。\n
    */
    bool Receive(void * data, uint32_t size_to_recv, uint32_t & transfered);

    /*! 同步（阻塞）接收数据
    @param[in] data           接收数据的缓冲区。
    @param[in] size_to_recv   期望接收的数据的大小。
    @param[in] timeout        接收的超时时间。
    @param[out] transfered    当前接收的数据的大小。
    @return 接收成功后返回true；否则返回false。
    @remark 做为TCP使用，用来在指定的时间内接收数据。\n
            如果发送方断开连接或者关闭，则transfered等于0。\n
    */
    bool Receive(void * data, uint32_t size_to_recv, 
                 uint32_t timeout, uint32_t & transfered);

    /*! 异步（非阻塞）接收数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步接收成功后返回true；否则返回false。
    @remark 做为TCP使用，用来接收数据。\n
            服务端使用上下文对象的accept_socket方法可以得到与客户端建立连接的Socket；
            客户端直接使用进行异步连接的Socket。\n
            上下文对象需要设置一个用于接收数据的缓冲区，并指定要接收的最大大小。
            同时，上下文对象还需要设置一个异步接受完成的回调函数，回调函数可以通过SocketAsyncEventAdapter进行适配，
            当异步接收完成时，在此回调函数中会返回该上下文对象，可以根据该上下文对象的transfered方法判断实际接收到的数据的大小。\n
    */
    bool ReceiveAsync(SocketAsyncContext & args);

    /*! 同步（阻塞）发送数据
    @param[in] data           发送数据的缓冲区。
    @param[in] size_to_recv   期望发送的数据的大小。
    @param[out] transfered    当前发送的数据的大小。
    @return 发送成功后返回true；否则返回false。
    @remark 做为TCP使用，用来发送数据。\n
            如果接收方断开连接或者关闭，transfered等于0。\n
    */
    bool Send(const void * data, uint32_t size_to_send, uint32_t & transfered);

    /*! 同步（阻塞）发送数据
    @param[in] data           发送数据的缓冲区。
    @param[in] size_to_recv   期望发送的数据的大小。
    @param[in] timeout        发送的超时时间。
    @param[out] transfered    当前发送的数据的大小。
    @return 发送成功后返回true；否则返回false。
    @remark 做为TCP使用，用来在指定的时间内发送数据。\n
            如果接收方断开连接或者关闭，transfered等于0。\n
    */
    bool Send(const void * data, uint32_t size_to_send, 
              uint32_t timeout, uint32_t & transfered);

    /*! 异步（非阻塞）发送数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步发送成功后返回true；否则返回false。
    @remark 做为TCP使用，用来发送数据。\n
            服务端使用上下文对象的accept_socket方法可以得到与客户端建立连接的Socket；
            客户端直接使用进行异步连接的Socket。\n
            上下文对象需要设置一个用于发送数据的缓冲区，并指定要发送数据的大小。
            同时，上下文对象还需要设置一个异步接受完成的回调函数，回调函数可以通过SocketAsyncEventAdapter进行适配，
            当异步发送完成时，在此回调函数中会返回该上下文对象，可以根据该上下文对象的count方法和transfered方法判断数据是否全部发送完毕。\n
    */
    bool SendAsync(SocketAsyncContext & args);

    /*! 同步（阻塞）接收数据
    @param[in] data           接收数据的缓冲区。
    @param[in] size_to_recv   期望接收的数据的大小。
    @param[out] transfered    当前接收的数据的大小。
    @param[out] endpoint      数据来源的地址。
    @return 接收成功后返回true；否则返回false。
    @remark 做为UDP使用，用来接收数据。\n
            如果发送方断开连接或者关闭，transfered等于0。\n
    */
    bool ReceiveFrom(void * data, uint32_t size_to_recv, 
                     uint32_t & transfered, IPEndPoint & endpoint);

    /*! 同步（阻塞）接收数据
    @param[in] data           接收数据的缓冲区。
    @param[in] size_to_recv   期望接收的数据的大小。
    @param[in] timeout        接收的超时时间。
    @param[out] transfered    当前接收的数据的大小。
    @param[out] endpoint      数据来源的地址。
    @return 接收成功后返回true；否则返回false。
    @remark 做为UDP使用，用来在指定的时间内接收数据。\n
            如果发送方断开连接或者关闭，transfered等于0。\n
    */
    bool ReceiveFrom(void * data, uint32_t size_to_recv, uint32_t timeout,
                     uint32_t & transfered, IPEndPoint & endpoint);

    /*! 异步（非阻塞）接收数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步接收成功后返回true；否则返回false。
    @remark 做为UDP使用，用来接收数据。\n
            在此之前，需要将当前套接字与一个Proactor进行关联。\n
            上下文对象需要设置用于接收到数据的客户端的IPEndPoint。
            同时，上下文对象还需要设置一个用于接收数据的缓冲区，并指定要接收的最大大小。
            另外，上下文对象还需要设置一个异步接受完成的回调函数，回调函数可以通过SocketAsyncEventAdapter进行适配，
            当异步接收完成时，在此回调函数中会返回该上下文对象，可以根据该上下文对象的transfered方法判断实际接收到的数据的大小。\n
    */
    bool ReceiveFromAsync(SocketAsyncContext & args);

    /*! 同步（阻塞）发送数据
    @param[in] data           发送数据的缓冲区。
    @param[in] size_to_recv   期望发送的数据的大小。
    @param[in] endpoint       数据发送的地址。
    @param[out] transfered    当前发送的数据的大小。
    @return 发送成功后返回true；否则返回false。
    @remark 做为UDP使用，用来发送数据。\n
            如果接收方断开连接或者关闭，transfered等于0。\n
    */
    bool SendTo(const void * data, uint32_t size_to_send, 
               const IPEndPoint & endpoint, uint32_t & transfered);

    /*! 同步（阻塞）发送数据
    @param[in] data           发送数据的缓冲区。
    @param[in] size_to_recv   期望发送的数据的大小。
    @param[in] endpoint       数据发送的地址。
    @param[in] timeout        发送的超时时间。
    @param[out] transfered    当前发送的数据的大小。
    @return 发送成功后返回true；否则返回false。
    @remark 做为UDP使用，用来在指定的时间内发送数据。\n
            如果接收方断开连接或者关闭，transfered等于0。\n
    */
    bool SendTo(const void * data, uint32_t size_to_send, 
                const IPEndPoint & endpoint, uint32_t timeout,
                uint32_t & transfered);

    /*! 异步（非阻塞）发送数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步发送成功后返回true；否则返回false。
    @remark 做为UDP使用，用来发送数据。\n
            在此之前，需要将当前套接字与一个Proactor进行关联。\n
            上下文对象需要设置用于发送到的服务端的IPEndPoint。
            同时，上下文对象还需要设置一个用于发送数据的缓冲区，并指定要发送数据的大小。
            另外，上下文对象还需要设置一个异步接受完成的回调函数，回调函数可以通过SocketAsyncEventAdapter进行适配，
            当异步发送完成时，在此回调函数中会返回该上下文对象，可以根据该上下文对象的count方法和transfered方法判断数据是否全部发送完毕。\n
    */
    bool SendToAsync(SocketAsyncContext & args);

    /*! 判断套接字是否可读
    @return 可读返回true；否则返回false。
    @remark 如果套接字处于Listen状态，当返回值为true时，此时使用Accept将保证是非阻塞的；\n
            其他情况下返回true时，此时使用Receive/ReceiveAsync，ReceiveFrom/ReceiveFromAsync将保证是非阻塞的。\n
    */
    bool CanRead();

    /*! 判断套接字是否可写
    @return 可写返回true；否则返回false。
    @remark 如果套接字处于异步（非阻塞）连接状态，当返回值为true时，意味着连接成功；\n
            其他情况下返回true时，此时使用Send/SendAsync，SendTo/SendToAsync能保证返回true。\n
    */
    bool CanWrite();

    /*! 判断套接字是否有效
    @return 有效返回true；否则返回false。
    @remark 如果返回false，则使用其他方法也将返回false。\n
    */
    bool IsValid();

    /*! 取消套接字的操作
    @return 取消成功返回true；否则返回false。
    */
    bool Cancel();

    /*! 关联到前摄器
    @param[in] io 前摄器对象。
    @return 关联成功返回true；否则返回false。
    @remark 将套接字绑定到完成端口上，用以进行异步操作。\n
            当套接字与前摄器成功关联后，在主循环中调用前摄器的Run方法即可进行异步事件的处理。\n
    */
    bool Associate(Proactor & io);

private:
    void * GetPlatformHandle();

    void OnCompleted(AsyncContext & args,
                     uint32_t error, 
                     uint32_t transfered);

    bool WaitSocketAsyncEvent(SocketAsyncContext & args);

private:
    Proactor * io_handler_;
    SOCKET s_;
};

}

#endif NCORE_SYS_SOCKET_H_