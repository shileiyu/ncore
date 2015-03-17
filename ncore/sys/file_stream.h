#ifndef NCORE_SYS_FILE_STREAM_H_
#define NCORE_SYS_FILE_STREAM_H_

#include <ncore/ncore.h>
#include <ncore/base/object.h>
#include <ncore/base/datetime.h>
#include "io_portal.h"
#include "file_define.h"

/*!
@file file_stream.h
*/
namespace ncore
{


class Proactor;
class FileStreamAsyncContext;

/*! 文件流类\n
可以实现两种模式的操作：\n
分为“同步（阻塞）”模式和“异步（非阻塞）”模式。\n
*/
class FileStream : public NonCopyableObject,
                   public IOPortal
{
#if defined NCORE_WINDOWS
    typedef HANDLE HandleType;
#endif
public:
    /*! 进入alertable状态
    @remark 主动使thread进入alertable状态,以便调用IOCR。
    */
    static void InvokeIOCompleteRoution();

public:
    FileStream();
    ~FileStream();

    /*! 
    @remark FileStream class support Rvalue references and move constructors。
    */
    FileStream(FileStream && obj);
    FileStream & operator = (FileStream && obj);

    /*! 初始化
    @param[in] fullname             文件名称。
    @param[in] access               访问方式。
    @param[in] share                共享方式。
    @param[in] mode                 文件模式。
    @param[in] maxattr_instances    文件属性。
    @param[in] option               文件设置。
    @return 初始化成功后返回true；否则返回false。
    */
    bool init(const char * filename, 
              FileAccess access,
              FileShare share,
              FileMode mode,
              FileAttribute attr,
              FileOption option);

    /*! 反初始化
    */
    void fini();

    /*! 刷新缓冲区
    @return 刷新成功后返回true；否则返回false。
    */
    bool Flush();

    /*! 设置文件大小, (已废弃)
    @param[in] size 大小。
    @return 设置成功后返回true；否则返回false。
    */
    bool SetFileSize(uint64_t size);

    /*! 将当前位置设置为文件尾
    @return 设置成功后返回true；否则返回false。
    */
    bool Truncate();

    /*! 定位指针位置
    @param[in] position         定位位置。
    @param[in] file_position    文件位置。
    @return 定位成功后返回true；否则返回false。
    @remark 可以将指针定位到超过文件大小的位置，但是不能将指针定位到小于文件起始的位置。\n
    */
    bool Seek(int64_t & position, FilePosition file_position);

    bool SetFilePos(uint64_t position);

    /*! 获得指针位置
    @param[out] pos 位置。
    @return 获得成功后返回true；否则返回false。
    */
    bool Tell(uint64_t & pos) const;

    /*! 获得文件大小
    @param[out] file_size 大小。
    @return 获得成功后返回true；否则返回false。
    */
    bool GetFileSize(uint64_t & file_size) const;

    /*! 同步（阻塞）读取数据
    @param[in] data             读取数据的缓冲区。
    @param[in] size_to_read     期望读取的数据的大小。
    @param[out] transfered      当前读取的数据的大小。
    @return 读取成功后返回true；否则返回false。
    */
    bool Read(void * data, uint32_t size_to_read, uint32_t & transfered);

    /*! 同步（阻塞）读取数据
    @param[in] data             读取数据的缓冲区。
    @param[in] size_to_read     期望读取的数据的大小。
    @param[in] offset           偏移位置。
    @param[out] transfered      当前读取的数据的大小。
    @return 读取成功后返回true；否则返回false。
    @remark 如果偏移位置等于或者超出文件尾，能读取成功，但是读取到的数据大小为0。\n
    */
    bool Read(void * data, uint32_t size_to_read, uint64_t offset, 
              uint32_t & transfered);

    /*! 异步（非阻塞）读取数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步读取成功后返回true；否则返回false。
    @remark 上下文对象需要设置一个用于读取数据的缓冲区，并指定要读取的最大大小。
            同时，上下文对象还需要设置一个异步读取完成的回调函数，回调函数可以通过FileStreamAsyncEventAdapter进行适配，
            当异步读取完成时，在此回调函数中会返回该上下文对象，可以根据该上下文对象的transfered方法判断实际读取到的数据的大小。\n
    */
    bool ReadAsync(FileStreamAsyncContext & args);

    /*! 同步（阻塞）写入数据
    @param[in] data             写入数据的缓冲区。
    @param[in] size_to_write    期望写入的数据的大小。
    @param[out] transfered      当前写入的数据的大小。
    @return 写入成功后返回true；否则返回false。
    */
    bool Write(const void * data, uint32_t size_to_write, uint32_t & transfered);

    /*! 同步（阻塞）写入数据
    @param[in] data             写入数据的缓冲区。
    @param[in] size_to_write    期望写入的数据的大小。
    @param[in] offset           偏移位置。
    @param[out] transfered      当前写入的数据的大小。
    @return 写入成功后返回true；否则返回false。
    @remark 偏移位置可以超出文件尾，并且能成功写入数据。\n
    */
    bool Write(const void * data, uint32_t size_to_write, uint64_t offset,
                uint32_t & transfered);

    /*! 异步（非阻塞）写入数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步写入成功后返回true；否则返回false。
    @remark 上下文对象需要设置一个用于写入数据的缓冲区，并指定要写入数据的大小。
            同时，上下文对象还需要设置一个异步写入完成的回调函数，回调函数可以通过FileStreamAsyncEventAdapter进行适配，
            当异步写入完成时，在此回调函数中会返回该上下文对象，可以根据该上下文对象的count方法和transfered方法判断数据是否全部写入完毕。\n
    */
    bool WriteAsync(FileStreamAsyncContext & args);

    /*! 同步（阻塞）锁定数据
    @param[in] offset   偏移位置。
    @param[in] size     大小。
    @return 锁定成功后返回true；否则返回false。
    @remark 默认以独占方式锁定。\n
            偏移位置可以超出文件尾，并且能成功锁定数据。\n
    */
    bool LockFile(uint64_t offset, uint64_t size);

    /*! 同步（阻塞）锁定数据
    @param[in] offset       偏移位置。
    @param[in] size         大小。
    @param[in] lock_mode    锁定模式。
    @return 锁定成功后返回true；否则返回false。
    */
    bool LockFile(uint64_t offset, uint64_t size, 
                  FileLockMode lock_mode);

    /*! 异步（非阻塞）锁定数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步锁定成功后返回true；否则返回false。
    @remark 上下文对象需要设置锁定的偏移位置，并指定要锁定的大小，以及锁定的模式。
            同时，上下文对象还需要设置一个异步写入完成的回调函数，回调函数可以通过FileStreamAsyncEventAdapter进行适配，
            当异步锁定完成时，在此回调函数中会返回该上下文对象。\n
            如果锁定能同步完成，则无返回值。\n
    */
    bool LockFileAsync(FileStreamAsyncContext & args);

    /*! 同步（阻塞）解锁数据
    @param[in] offset   偏移位置。
    @param[in] size     大小。
    @return 解锁成功后返回true；否则返回false。
    */
    bool UnlockFile(uint64_t offset, uint64_t size);

    /*! 异步（非阻塞）解锁数据
    @param[in] args 异步操作的上下文对象。
    @return 发起异步解锁成功后返回true；否则返回false。
    @remark 上下文对象需要设置解锁的偏移位置，并指定要解锁的大小。
            同时，上下文对象还需要设置一个异步写入完成的回调函数，回调函数可以通过FileStreamAsyncEventAdapter进行适配，
            当异步解锁完成时，在此回调函数中会返回该上下文对象。\n
            如果解锁能同步完成，则无返回值。\n
    */
    bool UnlockFileAsync(FileStreamAsyncContext & args);

    /*! 设置创建时间
    @param[in] dt 时间。
    @return 设置成功后返回true；否则返回false。
    */
    bool SetCreationTime(const DateTime & dt);

    /*! 获得创建时间
    @param[out] dt 时间。
    @return 获得成功后返回true；否则返回false。
    */
    bool GetCreationTime(DateTime & dt);

    /*! 设置最后访问时间
    @param[in] dt 时间。
    @return 设置成功后返回true；否则返回false。
    */
    bool SetLastAccessTime(const DateTime & dt);

    /*! 获得最后访问时间
    @param[out] dt 时间。
    @return 获得成功后返回true；否则返回false。
    */
    bool GetLastAccessTime(DateTime & dt);

    /*! 设置最后写入时间
    @param[in] dt 时间。
    @return 设置成功后返回true；否则返回false。
    */
    bool SetLastWriteTime(const DateTime & dt);

    /*! 获得最后写入时间
    @param[out] dt 时间。
    @return 获得成功后返回true；否则返回false。
    */
    bool GetLastWriteTime(DateTime & dt);

    /*! 判断是否有效
    @return 有效返回true；否则返回false。
    @remark 如果返回false，则使用其他方法也将返回false。\n
    */
    bool IsValid();

    /*! 取消操作
    @return 取消成功返回true；否则返回false。
    */
    bool Cancel();

    /*! 关联到前摄器
    @param[in] io 前摄器对象。
    @return 关联成功返回true；否则返回false。
    @remark 将文件流绑定到完成端口上，用以进行异步操作。\n
            当管道文件流与前摄器成功关联后，在主循环中调用前摄器的Run方法即可进行异步事件的处理。\n
    */
    bool Associate(Proactor & io);

private:
    void * GetPlatformHandle();

    void OnCompleted(AsyncContext & args,
                     uint32_t error, 
                     uint32_t transfered);

    bool WaitFileStreamAsyncEvent(FileStreamAsyncContext & args);

private:
    HandleType handle_;
    Proactor * io_handler_;
};


}
#endif
