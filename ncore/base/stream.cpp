#include "stream.h"

namespace ncore
{


namespace
{

size_t RoundUp(size_t size)
{
    static size_t kAlign = 0x1000;
    size_t remain = size % kAlign;

    return remain ? size + kAlign - remain : size ? size : kAlign;
}

}

MemoryStream::MemoryStream()
    : data_(0),
      capacity_(0),
      cur_pos_(0),
      valid_(0),
      limit_(0)
{
}

MemoryStream::~MemoryStream()
{
    Dispose();
}

bool MemoryStream::Write(const void * blob, uint32_t blob_size, 
                         uint32_t & transfered)
{
    transfered = 0;

    if(blob == 0)    
        return false;

    if(blob_size == 0)
        return true;

    size_t amount = cur_pos_ + blob_size;
    //Set a cap if there is a limit.
    if(limit_ != 0 && amount > limit_)
        amount = limit_;
    //realloc if we don't ave enough memory
    if(amount > capacity_)
        if(!ReAlloc(RoundUp(amount)))
            return false;

    size_t copy_size = amount - cur_pos_;
    memcpy(data_ + cur_pos_, blob, copy_size);
    cur_pos_ += copy_size;
    if(valid_ < cur_pos_)
        valid_ = cur_pos_;

    transfered = copy_size;
    return true;
}

bool MemoryStream::Read(void * blob, uint32_t blob_size,
                        uint32_t & transfered)
{
    transfered = 0;

    if(blob == 0)
        return false;

    if(blob_size == 0)
        return true;

    if(cur_pos_ >= valid_)
        return true;

    uint32_t avail = valid_ - cur_pos_;
    size_t size_to_read = std::min(avail, blob_size);
    memcpy(blob, data_ + cur_pos_, size_to_read);
    cur_pos_ += size_to_read;
    transfered = size_to_read;
    return true;
}

bool MemoryStream::Seek(int offset, StreamPosition::Value position)
{
    size_t target_pos = 0;
    switch(position)
    {
    case StreamPosition::kBegin:
        target_pos = offset;
        break;
    case StreamPosition::kCurrent:
        target_pos = cur_pos_ +  offset;
        break;
    case StreamPosition::kEnd:
        target_pos = valid_ + offset;
        break;
    default:
         return false;
    }

    /*It should not seek beyond the limit*/
    if(limit_ != 0 && limit_ < target_pos)
        return false;
    cur_pos_ = target_pos;
    return true;
}

size_t MemoryStream::Tell() const
{
    return cur_pos_;
}

bool MemoryStream::SetEndOfStream(size_t length)
{
    if(length > capacity_)
        if(!ReAlloc(RoundUp(length)))
            return false;

    valid_ = length;
    return true;
}

const void * MemoryStream::data() const
{
    return data_;
}

size_t MemoryStream::valid() const
{
    return valid_;
}

bool MemoryStream::ReAlloc(size_t size)
{
    data_ = reinterpret_cast<char *>(realloc(data_, size));
    if(data_ == 0)
        return false;

    capacity_ = size;
    return true;
}

void MemoryStream::Dispose()
{
    if(data_)
    {
        free(data_);
        data_ = 0;
    }
    valid_ = 0;
    capacity_ = 0;
    cur_pos_ = 0;
    limit_ = 0;
}


}