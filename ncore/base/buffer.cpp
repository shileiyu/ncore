#include "buffer.h"

namespace ncore
{


Buffer::Buffer()
    : data_(0),
      capacity_(0)
{
}

Buffer::~Buffer()
{
    Dispose();
}

Buffer::Buffer(size_t capacity)
    : data_(0),
      capacity_(0)
{
    Alloc(capacity);
}

Buffer::Buffer(const Buffer & obj)
    : data_(0),
      capacity_(0)
{
    CopyFrom(obj);
}

Buffer & Buffer::operator=(const Buffer & obj)
{
    CopyFrom(obj);
    return *this;
}

Buffer::Buffer(Buffer && obj)
    : data_(0),
      capacity_(0)
{
    std::swap(data_, obj.data_);
    std::swap(capacity_, obj.capacity_);
}

Buffer & Buffer::operator=(Buffer && obj)
{
    std::swap(data_, obj.data_);
    std::swap(capacity_, obj.capacity_);
    return *this;
}

const char * Buffer::data() const
{
    return data_;
}

char * Buffer::data()
{
    return data_;
}

size_t Buffer::capacity() const
{
    return capacity_;
}

Buffer::operator const char * () const
{
    return data_;
}

Buffer::operator char * ()
{
    return data_;
}

int Buffer::SetContent(const void * blob, size_t size)
{
    int copy_size = 0;
    //指针无效或者尺寸为0
    if(!blob || !size || !capacity_ || !data_)
        return 0;

    if(size > capacity_)
        copy_size = capacity_;
    else
        copy_size = size;

    char * dst_start = data_;
    char * dst_end = dst_start + copy_size;
    const char * src_start = reinterpret_cast<const char *>(blob);
    const char * src_end = src_start + copy_size;

    if(src_start > src_end || dst_start > dst_end)
        return 0;
    if( dst_start >= src_start && dst_start < src_end)
        return 0;
    if( dst_end > src_start && dst_end <= src_end )
        return 0;

    memcpy(dst_start, src_start, copy_size);

    return copy_size;
}

bool Buffer::CopyFrom(const Buffer & obj)
{
    size_t copy_size = obj.capacity_;
    char * copy_src = obj.data_;
    bool succed = true;

    Dispose();

    if(copy_src != 0)
    {
        if(Alloc(copy_size))
            memcpy(data_, copy_src, copy_size);
        else
            succed = false;        
    }
    return succed;
}

bool Buffer::Alloc(size_t capacity)
{
    Dispose();

    if(data_ = (char *)malloc(capacity))
        capacity_ = capacity;

    return data_ != NULL;
}

void Buffer::Dispose()
{
    if(data_)
    {
        free(data_);
        data_ = 0;
    }
    capacity_ = 0;
}


}