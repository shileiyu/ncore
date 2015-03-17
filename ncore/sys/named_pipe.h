#ifndef NCORE_SYS_NAMED_PIPE_H_
#define NCORE_SYS_NAMED_PIPE_H_

#include <ncore/ncore.h>
#include <ncore/base/object.h>
#include "io_portal.h"
#include "pipe_define.h"

/*!
@file named_pipe.h
*/
namespace ncore
{


class NamedPipeAsyncContext;

/*! 命名管道类\n
可以创建两种方式的命名管道：\n
一种是Stream，即字节流方式，数据以连续的字节流进行传输；\n
另一种是Message，即消息方式，数据以一系列不连续的数据单位进行传输。\n
通信模式分为“同步（阻塞）”模式和“异步（非阻塞）”模式。\n
*/
class NamedPipe : public NonCopyableObject,
                  public IOPortal
{
public:
    /*! 进入alertable状态
    @remark 主动使thread进入alertable状态,以便调用IOCR。
    */
    static void InvokeIOCompleteRoution();

#ifdef  NCORE_WINDOWS
    typedef HANDLE HandleType;
#endif

protected:
    NamedPipe();
    ~NamedPipe();

public:
    /*! 同步（阻塞）读取数据
    @param[in] buffer           读取数据的缓冲区。
    @param[in] size_to_read     期望读取的数据的大小。
    @param[out] transfered      当前读取的数据的大小。
    @return 读取成功后返回true；否则返回false。
    @remark 如果写入方断开连接，则transfered等于0。\n
    */
    bool Read(void * buffer, uint32_t size_to_read, uint32_t & transfered);

    /*! 同步（阻塞）读取数据
    @param[in] buffer           读取数据的缓冲区。
    @param[in] size_to_read     期望读取的数据的大小。
    @param[in] timeout          读取的超时时间。
    @param[out] transfered      当前读取的数据的大小。
    @return 读取成功后返回true；否则返回false。
    @remark 在指定的时间内读取数据。\n
            如果写入方断开连接，则transfered等于0。\n
    */
    bool Read(void * buffer, uint32_t size_to_read, uint32_t timeout, 
              uint32_t & transfered);

    /*! 异步（非阻塞）读取数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步读取成功后返回true；否则返回false。
    @remark 上下文对象需要设置一个用于读取数据的缓冲区，并指定要读取的最大大小。
            同时，上下文对象还需要设置一个异步读取完成的回调函数，回调函数可以通过NamedPipeAsyncEventAdapter进行适配，
            当异步读取完成时，在此回调函数中会返回该上下文对象，可以根据该上下文对象的transfered方法判断实际读取到的数据的大小。\n
    */
    bool ReadAsync(NamedPipeAsyncContext & args);

    /*! 同步（阻塞）写入数据
    @param[in] buffer           写入数据的缓冲区。
    @param[in] size_to_write    期望写入的数据的大小。
    @param[out] transfered      当前写入的数据的大小。
    @return 写入成功后返回true；否则返回false。
    @remark 如果读取方断开连接，transfered等于0。\n
    */
    bool Write(const void * buffer, uint32_t size_to_write, 
               uint32_t & transfered);

    /*! 同步（阻塞）写入数据
    @param[in] buffer           写入数据的缓冲区。
    @param[in] size_to_write    期望写入的数据的大小。
    @param[in] timeout          写入的超时时间。
    @param[out] transfered      当前写入的数据的大小。
    @return 写入成功后返回true；否则返回false。
    @remark 如果读取方断开连接，transfered等于0。\n
    */
    bool Write(const void * buffer, uint32_t size_to_write, 
               uint32_t timeout, uint32_t & transfered);

    /*! 异步（非阻塞）写入数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步写入成功后返回true；否则返回false。
    @remark 上下文对象需要设置一个用于写入数据的缓冲区，并指定要写入数据的大小。
            同时，上下文对象还需要设置一个异步写入完成的回调函数，回调函数可以通过NamedPipeAsyncEventAdapter进行适配，
            当异步写入完成时，在此回调函数中会返回该上下文对象，可以根据该上下文对象的count方法和transfered方法判断数据是否全部写入完毕。\n
    */
    bool WriteAsync(NamedPipeAsyncContext & args);

    /*! 预览数据
    @param[in] buffer           预览时要读取数据的缓冲区，如果不读取，传入NULL。
    @param[in] size_to_read     期望读取的数据的大小。
    @param[out] transfered      当前读取的数据的大小。
    @param[out] bytes_avail     管道中数据的总大小。
    @param[out] bytes_left_this_message     管道中剩余数据的大小。
    @return 预览成功后返回true；否则返回false。
    @remark 在同步（阻塞）操作的时候，可以先预览管道中的数据大小，防止阻塞。\n
    */
    bool Peek(void * buffer, uint32_t size_to_read, uint32_t & transfered,
              uint32_t & bytes_avail, uint32_t & bytes_left_this_message);

    /*! 取消操作
    @return 取消成功返回true；否则返回false。
    */
    bool Cancel();

    /*! 判断是否有效
    @return 有效返回true；否则返回false。
    @remark 如果返回false，则使用其他方法也将返回false。\n
    */
    bool IsValid() const;

    /*! 关联到前摄器
    @param[in] io 前摄器对象。
    @return 关联成功返回true；否则返回false。
    @remark 将管道绑定到完成端口上，用以进行异步操作。\n
            当管道与前摄器成功关联后，在主循环中调用前摄器的Run方法即可进行异步事件的处理。\n
    */
    bool Associate(Proactor & io);

protected:
    void * GetPlatformHandle();

    void OnCompleted(AsyncContext & args,
                     uint32_t error,
                     uint32_t transfered);

    bool WaitNamedPipeAsyncEvent(NamedPipeAsyncContext & args);

protected:
    HandleType handle_;
    Proactor * io_handler_;
};

class NamedPipeServer : public NamedPipe
{
public:
    NamedPipeServer();
    ~NamedPipeServer();

    //move semantic
    NamedPipeServer(NamedPipeServer && obj);
    NamedPipeServer & operator = (NamedPipeServer && obj);

    /*! 初始化，创建服务端管道
    @param[in] pipe_name        管道名称。
    @param[in] direction        传输方向。
    @param[in] option           控制模式。
    @param[in] transmission     传输模式。
    @param[in] max_instances    最大实例数。
    @param[in] out_buffer_size  读取缓冲区的大小。
    @param[in] in_buffer_size   写入缓冲区的大小。
    @param[in] timeout          超时时间。
    @return 初始化成功后返回true；否则返回false。
    */
    bool init(const char * pipe_name, 
              PipeDirection direction,
              PipeOption option,
              PipeTransmissionMode transmission,
              uint32_t max_instances,
              uint32_t out_buffer_size,
              uint32_t in_buffer_size,
              uint32_t timeout);

    /*! 反初始化，关闭管道
    */
    void fini();

    /*! 同步（阻塞）接受连接
    @return 接受成功后返回true；否则返回false。
    */
    bool Accept();

    /*! 同步（阻塞）接受连接
    @param[in] timeout 超时时间。
    @return 接受成功后返回true；否则返回false。
    */
    bool Accept(uint32_t timeout);

    /*! 异步（非阻塞）接受连接
    @param[in] args 异步操作的上下文对象。
    @return 发起异步接受成功后返回true；否则，返回false。
    @remark 在此之前，需要将当前管道与一个Proactor进行关联。\n
            上下文对象需要设置一个异步接受完成的回调函数，回调函数可以通过NamedPipeAsyncEventAdapter进行适配，
            当异步接受完成时，在此回调函数中会返回该上下文对象。\n
    */
    bool AcceptAsync(NamedPipeAsyncContext & args);

    /*! 断开连接
    @return 断开成功后返回true；否则返回false。
    */
    bool Disconnect();
};

class NamedPipeClient : public NamedPipe
{
public:
    NamedPipeClient();
    ~NamedPipeClient();

    //move semantic
    NamedPipeClient(NamedPipeClient && obj);
    NamedPipeClient & operator = (NamedPipeClient && obj);

    /*! 初始化，创建客户端管道
    @param[in] pipe_name    管道名称。
    @param[in] direction    传输方向。
    @param[in] option       控制模式。
    @param[in] timeout      超时时间。
    @return 初始化成功后返回true；否则返回false。
    */
    bool init(const char * pipe_name,
              PipeDirection direction,
              PipeOption option,
              uint32_t timeout);

    /*! 反初始化，关闭管道
    */
    void fini();
};


}
#endif