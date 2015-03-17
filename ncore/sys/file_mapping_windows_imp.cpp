#include "file_mapping.h"

namespace ncore
{
    

FileMapping::FileMapping()
{
    handle_ = 0;
}

FileMapping::~FileMapping()
{
}

bool FileMapping::init(const Mode mode, const uint64_t size)
{
    uint32_t flag = 0;
    uint32_t sizelo = 0;
    uint32_t sizehi = 0;

    if(mode == kRead)
    {
        flag = PAGE_READONLY;
    }
    else if(mode == kReadWrite)
    {
        flag = PAGE_READWRITE;
    }
    else
    {
        assert(0);
    }

    sizelo = static_cast<uint32_t>(size);
    sizehi = static_cast<uint32_t>(size >> 32);

    handle_ = CreateFileMapping(INVALID_HANDLE_VALUE, 0, flag, sizehi, sizelo, 0);

    if(handle_)
        return true;
    else
        return false;
}

void FileMapping::fini()
{
    if(handle_)
    {
        CloseHandle(handle_);
        handle_ = 0;
    }
}

void * FileMapping::handle() const
{
    return handle_;
}


}