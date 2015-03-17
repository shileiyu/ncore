#ifndef NCORE_SYS_FILE_STREAM_DEFINE_H_
#define NCORE_SYS_FILE_STREAM_DEFINE_H_

#include <ncore/utils/bitwise_enum.h>

namespace ncore
{

struct _FileAccess
{
    enum Enum
    {
        kAll = GENERIC_ALL,
        kExecute = GENERIC_EXECUTE,
        kRead = GENERIC_READ,
        kWrite = GENERIC_WRITE,
        kReadWrite = kRead | kWrite,
    };
};

using FileAccess = BitwiseEnum<_FileAccess>;

enum FileMode {
    kCreateAlways = CREATE_ALWAYS,
    kCreateNew = CREATE_NEW,
    kOpen = OPEN_EXISTING,
    kOpenOrCreate = OPEN_ALWAYS,
    kTruncate = TRUNCATE_EXISTING,
};


struct _FileShare
{
    enum Enum
    {
        kExclusive = 0,
        kShareDelete = FILE_SHARE_DELETE,
        kShareRead = FILE_SHARE_READ,
        kShareWrite = FILE_SHARE_WRITE,
        kShareReadWrite = kShareRead | kShareWrite,
    };
};

using FileShare = BitwiseEnum<_FileShare>;

struct _FileOption
{
    enum Enum
    {
        kNone = 0,
        kDeleteOnClose = FILE_FLAG_DELETE_ON_CLOSE,
        kRandomAccess = FILE_FLAG_RANDOM_ACCESS,
        kSequentialScan = FILE_FLAG_SEQUENTIAL_SCAN,
        kWriteThrough = FILE_FLAG_WRITE_THROUGH,
    };
};

using FileOption = BitwiseEnum<_FileOption>;

struct _FileAttribute
{
    enum Enum
    {
        kNormal = FILE_ATTRIBUTE_NORMAL,
        kHidden = FILE_ATTRIBUTE_HIDDEN,
        kReadOnly = FILE_ATTRIBUTE_READONLY,
        kSystem = FILE_ATTRIBUTE_SYSTEM,
        kTemporary = FILE_ATTRIBUTE_SYSTEM,
        kArchive = FILE_ATTRIBUTE_ARCHIVE,
    };
};

using FileAttribute = BitwiseEnum<_FileAttribute>;

enum FilePosition
{
    kBegin = FILE_BEGIN,
    kCurrent = FILE_CURRENT,
    kEnd = FILE_END,
};


struct _FileLockMode
{
    enum Enum
    {
        kExclusiveLock = LOCKFILE_EXCLUSIVE_LOCK,
        kFailImmediately = LOCKFILE_FAIL_IMMEDIATELY,
    };
};

using FileLockMode = BitwiseEnum<_FileLockMode>;


struct _FileChangesNotify
{
    enum Enum
    {
        kChangeFileName = FILE_NOTIFY_CHANGE_FILE_NAME,
        kChangeDirectoryName = FILE_NOTIFY_CHANGE_DIR_NAME,
        kChangeAttributes = FILE_NOTIFY_CHANGE_ATTRIBUTES,
        kChangeSize = FILE_NOTIFY_CHANGE_SIZE,
        kChangeLastWrite = FILE_NOTIFY_CHANGE_LAST_WRITE,
        kChangeLastAccess = FILE_NOTIFY_CHANGE_LAST_ACCESS,
        kChangeCreation = FILE_NOTIFY_CHANGE_CREATION,
        kChangeSecurity = FILE_NOTIFY_CHANGE_SECURITY,
        kWatchSubtree = 0x80000000,
    };
};

using FileChangesNotify = BitwiseEnum<_FileChangesNotify>;

}

#endif