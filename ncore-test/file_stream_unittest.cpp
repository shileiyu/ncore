#include <gtest\gtest.h>
#include <ncore/base/buffer.h>
#include <ncore/algorithm/md5.h>
#include <ncore/utils/handy.h>
#include <ncore/sys/thread.h>
#include <ncore/sys/file_stream.h>
#include <ncore/sys/proactor.h>
#include <ncore/sys/file_stream_async_event_args.h>

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


// 异步操作的ClientContext
class ClientContext
{
public:
    static const size_t kMaxBufferSize = 204800;    // 200KB

public:
    ClientContext();
    ~ClientContext();

    FileStreamAsyncContext & arg();
    char * buffer();

private:
    FileStreamAsyncContext arg_;
    char buffer_[kMaxBufferSize];
};

ClientContext::ClientContext()
{
}

ClientContext::~ClientContext()
{
}

FileStreamAsyncContext & ClientContext::arg()
{
    return arg_;
}

char * ClientContext::buffer()
{
    return buffer_;
}

// AsyncRWFileStream
class AsyncRWFileStream
{
public:
    static const size_t kMaxBufferSize = 204800;    // 200KB

public:
    AsyncRWFileStream();
    ~AsyncRWFileStream();

    bool init();
    void fini();

    bool is_completed() const;

    void SetWriteBuffer(const void * buffer, size_t size);

    const void * write_buffer();
    size_t write_buffer_size();

    const void * read_buffer();
    size_t read_buffer_size();

    bool StartWrite();

    void IOProc();

private:
    void OnIOCompleted(FileStreamAsyncContext & ctx);
    void ProcessWrite(FileStreamAsyncContext & ctx);
    void ProcessRead(FileStreamAsyncContext & ctx);

private:
    Proactor proactor_;
    FileStreamAsyncResultAdapter<AsyncRWFileStream> event_adapter_;

    FileStream fs_;
    ClientContext client_context_;

    char write_buffer_[kMaxBufferSize];
    char read_buffer_[kMaxBufferSize];
    size_t write_buffer_size_;
    size_t read_buffer_size_;
    size_t size_writed_;

    bool is_completed_;
    size_t io_count_;
};

AsyncRWFileStream::AsyncRWFileStream()
    : write_buffer_size_(0),
    read_buffer_size_(0),
    size_writed_(0),
    is_completed_(false),
    io_count_(0)
{
    event_adapter_.Register(this, &AsyncRWFileStream::OnIOCompleted);
}

AsyncRWFileStream::~AsyncRWFileStream()
{
}

bool AsyncRWFileStream::init()
{
    bool succeed = false;

    succeed = proactor_.init();
    if (!succeed) return false;

    succeed = fs_.init("file_stream_asyncrw",
        FileAccess::kReadWrite,
        FileShare::kExclusive,
        FileMode::kCreateAlways,
        FileAttribute::kNormal,
        FileOption::kDeleteOnClose);
    if (!succeed) return false;

    succeed = fs_.Associate(proactor_);
    if (!succeed) return false;

    return true;
}

void AsyncRWFileStream::fini()
{
    fs_.fini();

    while (io_count_) IOProc();

    is_completed_ = true;

    proactor_.fini();
}

bool AsyncRWFileStream::is_completed() const
{
    return is_completed_;
}

void AsyncRWFileStream::SetWriteBuffer(const void * buffer, size_t size)
{
    memset(write_buffer_, 0, kMaxBufferSize);
    write_buffer_size_ = kMaxBufferSize < size ? kMaxBufferSize : size;
    memcpy(write_buffer_, buffer, write_buffer_size_);
}

const void * AsyncRWFileStream::write_buffer()
{
    return write_buffer_;
}

size_t AsyncRWFileStream::write_buffer_size()
{
    return write_buffer_size_;
}

const void * AsyncRWFileStream::read_buffer()
{
    return read_buffer_;
}

size_t AsyncRWFileStream::read_buffer_size()
{
    return read_buffer_size_;
}

bool AsyncRWFileStream::StartWrite()
{
    is_completed_ = false;

    memset(read_buffer_, 0, kMaxBufferSize);
    read_buffer_size_ = 0;
    size_writed_ = 0;

    FileStreamAsyncContext & arg = client_context_.arg();
    arg.SetBuffer(write_buffer_, write_buffer_size_);
    arg.set_completion_delegate(&event_adapter_);

    bool succeed = fs_.WriteAsync(arg);
    if (!succeed)
        return false;
    else
        ++io_count_;

    return true;
}

void AsyncRWFileStream::IOProc()
{
    proactor_.Run(1);
}

void AsyncRWFileStream::OnIOCompleted(FileStreamAsyncContext & arg)
{
    switch (arg.last_op())
    {
    case AsyncFileStreamOp::kAsyncWrite:
    {
        ProcessWrite(arg);
    }
        break;
    case AsyncFileStreamOp::kAsyncRead:
    {
        ProcessRead(arg);
    }
        break;
    }

    --io_count_;
}

void AsyncRWFileStream::ProcessWrite(FileStreamAsyncContext & arg)
{
    if (arg.error() || arg.transfered() <= 0) return;

    size_writed_ += arg.transfered();
    size_t write_size = write_buffer_size_ - size_writed_;
    if (write_size)
    {
        arg.SetBuffer(write_buffer_ + size_writed_, write_size);
        bool succeed = fs_.WriteAsync(arg);
        if (!succeed)
            return;
        else
            ++io_count_;
    }

    arg.SetBuffer(read_buffer_ + read_buffer_size_, arg.transfered());
    bool succeed = fs_.ReadAsync(arg);
    if (!succeed)
        return;
    else
        ++io_count_;
}

void AsyncRWFileStream::ProcessRead(FileStreamAsyncContext & arg)
{
    if (arg.error() || arg.transfered() <= 0) return;

    is_completed_ = false;

    read_buffer_size_ += arg.transfered();
    size_t read_size = write_buffer_size_ - read_buffer_size_;
    if (read_size)
    {
        arg.SetBuffer(read_buffer_ + read_buffer_size_, read_size);
        bool succeed = fs_.ReadAsync(arg);
        if (!succeed)
            return;
        else
            ++io_count_;
    }
    else
    {
        is_completed_ = true;
    }
}

// AsyncLockFileStream
class AsyncLockFileStream
{
public:
    static const size_t kMaxBufferSize = 204800;    // 200KB

public:
    AsyncLockFileStream();
    ~AsyncLockFileStream();

    bool init(const std::string & file_name,
        const void * buffer, size_t size);
    void fini();

    bool is_completed() const;

    const void * write_buffer();
    size_t write_buffer_size();

    bool StartUnlockLock();
    void IOProc();

private:
    void OnIOCompleted(FileStreamAsyncContext & arg);
    void ProcessLock(FileStreamAsyncContext & arg);
    void ProcessUnlock(FileStreamAsyncContext & arg);

private:
    Proactor proactor_;
    FileStreamAsyncResultAdapter<AsyncLockFileStream> event_adapter_;

    FileStream fs_;
    ClientContext client_context_;

    char write_buffer_[kMaxBufferSize];
    size_t write_buffer_size_;

    bool is_completed_;
    size_t io_count_;
};

AsyncLockFileStream::AsyncLockFileStream()
    : write_buffer_size_(0)
    , is_completed_(false)
    , io_count_(0)
{
    event_adapter_.Register(this, &AsyncLockFileStream::OnIOCompleted);
}

AsyncLockFileStream::~AsyncLockFileStream()
{
}

bool AsyncLockFileStream::init(
    const std::string & file_name,
    const void * buffer, size_t size
    ) {

    bool succeed = false;

    succeed = proactor_.init();
    if (!succeed) return false;

    succeed = fs_.init(
        file_name.data(), FileAccess::kReadWrite,
        FileShare::kShareReadWrite, FileMode::kOpen,
        FileAttribute::kNormal, FileOption::kNone
    );
    if (!succeed) return false;

    succeed = fs_.Associate(proactor_);
    if (!succeed) return false;

    memset(write_buffer_, 0, kMaxBufferSize);
    write_buffer_size_ = kMaxBufferSize < size ? kMaxBufferSize : size;
    memcpy(write_buffer_, buffer, write_buffer_size_);
    uint32_t write_size = 0;
    succeed = fs_.Write(write_buffer_, write_buffer_size_, 0, write_size);
    if (!succeed) return false;
    if (write_buffer_size_ != write_size) return false;

    succeed = fs_.LockFile(0, write_buffer_size_);
    if (!succeed) return false;

    return true;
}

void AsyncLockFileStream::fini()
{
    fs_.fini();

    while (io_count_) Sleep(1);

    is_completed_ = true;

    proactor_.fini();
}

bool AsyncLockFileStream::is_completed() const
{
    return is_completed_;
}

const void * AsyncLockFileStream::write_buffer()
{
    return write_buffer_;
}

size_t AsyncLockFileStream::write_buffer_size()
{
    return write_buffer_size_;
}

bool AsyncLockFileStream::StartUnlockLock()
{
    FileStreamAsyncContext & arg = client_context_.arg();
    arg.set_completion_delegate(&event_adapter_);

    FileLockMode mode;
    mode |= FileLockMode::kExclusiveLock;
    mode |= FileLockMode::kFailImmediately;

    arg.set_offset(0);
    arg.set_lock_size(write_buffer_size_);
    arg.set_lock_mode(mode);

    bool succeed = fs_.UnlockFileAsync(arg);
    if (!succeed)
        return false;
    else
        ++io_count_;

    return true;
}

void AsyncLockFileStream::IOProc()
{
    proactor_.Run(1);
}

void AsyncLockFileStream::OnIOCompleted(FileStreamAsyncContext & arg)
{
    switch (arg.last_op())
    {
    case AsyncFileStreamOp::kAsyncLock:
    {
        ProcessLock(arg);
    }
        break;
    case AsyncFileStreamOp::kAsyncUnlock:
    {
        ProcessUnlock(arg);
    }
        break;
    }

    --io_count_;
}

void AsyncLockFileStream::ProcessLock(FileStreamAsyncContext & arg)
{
    if (arg.error() || arg.lock_size() <= 0) return;

    is_completed_ = true;
}

void AsyncLockFileStream::ProcessUnlock(FileStreamAsyncContext & arg)
{
    if (arg.error() || arg.lock_size() <= 0) return;

    bool succeed = fs_.LockFileAsync(arg);
    if (!succeed)
        return;
    else
        ++io_count_;
}

// FileStreamTest
class FileStreamTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        bool succeed = false;
        file_name_ = "file_stream";
        FileStream fs_write;
        succeed = fs_write.init(
            file_name_.data(), FileAccess::kWrite, FileShare::kExclusive,
            FileMode::kCreateAlways, FileAttribute::kNormal, FileOption::kNone
        );
        if (!succeed) return;

        for (size_t index = 0; index < countof(write_buffer_); ++index)
            write_buffer_[index] = GetTickCount();

        const uint32_t size_to_write = sizeof(write_buffer_);
        uint32_t write_size = 0;
        succeed = fs_write.Write(write_buffer_, size_to_write, write_size);
        if (!succeed) return;

        succeed = fs_write.GetFileSize(file_size_);
        if (size_to_write != file_size_) return;

        fs_write.fini();
    }

    static void TearDownTestCase()
    {
        FileStream fs_read;
        fs_read.init(
            file_name_.data(), FileAccess::kRead, FileShare::kExclusive,
            FileMode::kOpen, FileAttribute::kNormal, FileOption::kDeleteOnClose
        );
        fs_read.fini();
    }

protected:
    static std::string file_name_;
    static uint64_t file_size_;
    static int write_buffer_[51200];  // 200KB
};

std::string FileStreamTest::file_name_;
uint64_t FileStreamTest::file_size_;
int FileStreamTest::write_buffer_[51200];

// 同步读写测试
TEST_F(FileStreamTest, SyncIO)
{
    bool succeed = false;

    FileStream fs;
    succeed = fs.init(
        file_name_.data(), FileAccess::kReadWrite, FileShare::kExclusive,
        FileMode::kOpen, FileAttribute::kNormal, FileOption::kNone
        );
    ASSERT_TRUE(succeed);

    // SetCreationTime and GetCreationTime
    int64_t creation_tick = GetTickCount();
    DateTime set_creation_time(creation_tick);
    succeed = fs.SetCreationTime(set_creation_time);
    EXPECT_TRUE(succeed);
    DateTime get_creation_time;
    succeed = fs.GetCreationTime(get_creation_time);
    EXPECT_TRUE(succeed);
    EXPECT_TRUE(set_creation_time == get_creation_time);

    // SetLastAccessTime and GetLastAccessTime
    int64_t la_tick = GetTickCount();
    DateTime set_la_time(la_tick);
    succeed = fs.SetLastAccessTime(set_la_time);
    EXPECT_TRUE(succeed);
    DateTime get_la_time;
    succeed = fs.GetLastAccessTime(get_la_time);
    EXPECT_TRUE(succeed);
    EXPECT_TRUE(set_la_time == get_la_time);

    // SetLastWriteTime and GetLastWriteTime
    int64_t lw_tick = GetTickCount();
    DateTime set_lw_time(lw_tick);
    succeed = fs.SetLastWriteTime(set_lw_time);
    EXPECT_TRUE(succeed);
    DateTime get_lw_time;
    succeed = fs.GetLastWriteTime(get_lw_time);
    EXPECT_TRUE(succeed);
    EXPECT_TRUE(set_lw_time == get_lw_time);

    // SetFileSize
    file_size_ = file_size_ / 2;
    succeed = fs.SetFileSize(file_size_);
    EXPECT_TRUE(succeed);

    // Calc hash
    Hash write_buffer_hash;
    CalcHash(write_buffer_, static_cast<size_t>(file_size_), write_buffer_hash);

    // GetFileSize
    uint64_t read_file_size = 0;
    succeed = fs.GetFileSize(read_file_size);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(file_size_, read_file_size);

    // Seek
    int64_t read_file_seek_pos = 0;
    succeed = fs.Seek(read_file_seek_pos, FilePosition::kBegin);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(0, read_file_seek_pos);

    // Read
    int buffer[51200];  // 200KB
    char * read_buffer = reinterpret_cast<char *>(buffer);
    const uint64_t size_to_read = read_file_size;
    uint32_t read_size = 0;
    while (read_size < size_to_read)
    {
        uint32_t size = 0;
        char * dst = read_buffer + read_size;
        succeed = fs.Read(dst, 33, read_size, size);
        if (!succeed) break;
        read_size += size;
    }
    EXPECT_EQ(size_to_read, read_size);

    // Tell
    fs.Seek(read_file_seek_pos, FilePosition::kEnd);
    uint64_t read_file_pos = 0;
    succeed = fs.Tell(read_file_pos);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(size_to_read, read_file_pos);

    // Calc hash
    Hash read_buffer_hash;
    CalcHash(read_buffer, read_size, read_buffer_hash);

    EXPECT_TRUE(write_buffer_hash == read_buffer_hash);

    fs.fini();
}

// 异步读写测试
TEST_F(FileStreamTest, AsyncIO)
{
    bool succeed = false;

    AsyncRWFileStream afs;
    succeed = afs.init();
    ASSERT_TRUE(succeed);

    afs.SetWriteBuffer(write_buffer_, sizeof(write_buffer_));

    succeed = afs.StartWrite();
    ASSERT_TRUE(succeed);

    while (!afs.is_completed()) afs.IOProc();

    // Calculate hash of write buffer
    const void * write_buffer = afs.write_buffer();
    const size_t size_to_write = afs.write_buffer_size();
    Hash write_buffer_hash;
    CalcHash(write_buffer, size_to_write, write_buffer_hash);

    // Calculate hash of read buffer
    const void * read_buffer = afs.read_buffer();
    const size_t size_to_read = afs.read_buffer_size();
    Hash read_buffer_hash;
    CalcHash(read_buffer, size_to_read, read_buffer_hash);

    EXPECT_TRUE(write_buffer_hash == read_buffer_hash);

    afs.fini();
}

// 同步锁/解锁测试
TEST_F(FileStreamTest, SyncLockUnlock)
{
    bool succeed = false;

    FileStream fs_lock;
    succeed = fs_lock.init(
        file_name_.data(), FileAccess::kReadWrite,
        FileShare::kShareReadWrite, FileMode::kOpen,
        FileAttribute::kNormal, FileOption::kNone
    );
    ASSERT_TRUE(succeed);

    FileStream fs_read;
    succeed = fs_read.init(
        file_name_.data(), FileAccess::kReadWrite,
        FileShare::kShareReadWrite, FileMode::kOpen,
        FileAttribute::kNormal, FileOption::kNone
    );
    ASSERT_TRUE(succeed);

    uint64_t lock_size = file_size_;
    succeed = fs_lock.LockFile(0, lock_size);
    EXPECT_TRUE(succeed);
    succeed = fs_lock.UnlockFile(0, lock_size);
    EXPECT_TRUE(succeed);

    succeed = fs_lock.LockFile(file_size_, lock_size);
    EXPECT_TRUE(succeed);

    int buffer[51200];  // 200KB
    char * dst = reinterpret_cast<char *>(buffer);
    const uint32_t size_to_read = static_cast<const uint32_t>(file_size_);
    uint32_t read_size = 0;
    succeed = fs_read.Read(dst, size_to_read, file_size_, read_size);
    EXPECT_TRUE(!succeed);
    EXPECT_EQ(0, read_size);

    succeed = fs_lock.UnlockFile(file_size_, lock_size);
    EXPECT_TRUE(succeed);

    read_size = 0;
    succeed = fs_read.Read(dst, size_to_read, file_size_, read_size);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(0, read_size);

    fs_read.fini();
    fs_lock.fini();
}

// 异步锁/解锁测试
TEST_F(FileStreamTest, AsyncLockUnlock)
{
    bool succeed = false;

    AsyncLockFileStream alfs;
    succeed = alfs.init(file_name_, write_buffer_, sizeof(write_buffer_));
    ASSERT_TRUE(succeed);

    FileStream fs_read;
    succeed = fs_read.init(
        file_name_.data(), FileAccess::kReadWrite,
        FileShare::kShareReadWrite, FileMode::kOpen,
        FileAttribute::kNormal, FileOption::kNone
    );
    EXPECT_TRUE(succeed);

    int buffer[51200];  // 200KB
    char * dst = reinterpret_cast<char *>(buffer);
    const uint32_t size_to_read = sizeof(write_buffer_);
    uint32_t read_size = 0;
    succeed = fs_read.Read(dst, size_to_read, read_size);
    EXPECT_TRUE(!succeed);
    EXPECT_EQ(0, read_size);

    succeed = alfs.StartUnlockLock();
    EXPECT_TRUE(succeed);

    while (!alfs.is_completed()) alfs.IOProc();

    read_size = 0;
    succeed = fs_read.Read(dst, size_to_read, read_size);
    EXPECT_TRUE(!succeed);
    EXPECT_EQ(0, read_size);

    alfs.fini();
}

// 同步边界值测试
TEST_F(FileStreamTest, SyncBoundary)
{
    bool succeed = false;

    std::string file_name("file_stream_sync_boundary");
    FileStream fs;
    succeed = fs.init(
        file_name.data(),
        FileAccess::kReadWrite,
        FileShare::kExclusive,
        FileMode::kCreateAlways,
        FileAttribute::kNormal,
        FileOption::kDeleteOnClose
    );
    ASSERT_TRUE(succeed);

    // Write
    uint32_t size_to_write = sizeof(write_buffer_);
    uint32_t write_size = 0;
    succeed = fs.Write(write_buffer_, size_to_write, write_size);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(size_to_write, write_size);

    uint64_t write_offset = size_to_write + 2;
    write_size = 0;
    succeed = fs.Write(write_buffer_, size_to_write, write_offset, write_size);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(size_to_write, write_size);

    succeed = fs.SetFileSize(size_to_write);
    EXPECT_TRUE(succeed);

    // Read
    int read_buffer[51200];  // 200KB
    uint64_t read_offset = size_to_write;
    uint32_t read_size = 0;
    succeed = fs.Read(read_buffer, size_to_write, read_offset, read_size);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(0, read_size);

    read_offset = size_to_write + 2;
    read_size = 0;
    succeed = fs.Read(read_buffer, size_to_write, read_offset, read_size);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(0, read_size);

    // Lock
    uint64_t lock_offset = 0;
    uint64_t lock_size = size_to_write + 2;
    succeed = fs.LockFile(lock_offset, lock_size);
    EXPECT_TRUE(succeed);
    lock_offset = 0;
    succeed = fs.UnlockFile(lock_offset, lock_size);
    EXPECT_TRUE(succeed);

    // Seek
    int64_t position = -1;
    succeed = fs.Seek(position, FilePosition::kBegin);
    EXPECT_TRUE(!succeed);
    EXPECT_EQ(-1, position);

    position = size_to_write + 1;
    succeed = fs.Seek(position, FilePosition::kBegin);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(size_to_write + 1, position);

    int64_t size = size_to_write;
    position = -size - 1;
    succeed = fs.Seek(position, FilePosition::kEnd);
    EXPECT_TRUE(!succeed);
    EXPECT_EQ(-size - 1, position);

    position = 1;
    succeed = fs.Seek(position, FilePosition::kEnd);
    EXPECT_TRUE(succeed);
    EXPECT_EQ(size_to_write + 1, position);

    fs.fini();
}

}