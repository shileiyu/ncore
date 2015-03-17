#ifndef NCORE_BASE_STREAM_H_
#define NCORE_BASE_STREAM_H_


#include <ncore/ncore.h>

namespace ncore
{


namespace StreamPosition
{
enum Value
{
    kBegin,
    kCurrent,
    kEnd,
};
}

class MemoryStream
{
public:
    MemoryStream();
    ~MemoryStream();

    bool Write(const void * blob, uint32_t blob_size, uint32_t & transfered);
    bool Read(void * blob, uint32_t blob_size, uint32_t & transfered);
    bool Seek(int offset, StreamPosition::Value pos);
    size_t Tell() const;
    bool SetEndOfStream(size_t length);

    const void * data() const;
    size_t valid() const;
private:
    bool ReAlloc(size_t size);
    void Dispose();
private:
    char * data_;
    size_t cur_pos_;
    size_t valid_;
    size_t capacity_;
    size_t limit_;
};


}

#endif