#ifndef NCORE_BASE_BUFFER_H_
#define NCORE_BASE_BUFFER_H_

#include "object.h"

namespace ncore
{

class Buffer
{
public:
    Buffer();
    ~Buffer();

    Buffer(size_t capacity);

    Buffer(const Buffer & obj);
    Buffer & operator=(const Buffer & obj);

    Buffer(Buffer && obj);
    Buffer & operator=(Buffer && obj);

    const char * data() const;
    char * data();
    size_t capacity() const;

    operator const char * () const;
    operator char * ();

    int SetContent(const void * blob, size_t size);

    bool CopyFrom(const Buffer & obj);
private:
    bool Alloc(size_t capacity);
    void Dispose();
private:
    size_t capacity_;
    char * data_;
};

template<size_t length>
class FixedBuffer
{
public:
    FixedBuffer()
    {
        memset(data_, 0, sizeof(data_));
    }

    FixedBuffer(const FixedBuffer<length> & obj)
    {
        memcpy(data_, obj.data_, length);
    }

    ~FixedBuffer()
    {
    }

    const char * data() const
    {
        return data_;
    }

    char * data()
    {
        return data_;
    }

    void SetContent(const char * blob, int size)
    {
        if(size > sizeof(data_))
            memcpy(data_, blob, sizeof(data_));
        else
            memcpy(data_, blob, size);
    }

    operator const char *() const
    {
        return data_;
    }

    operator char *()
    {
        return data_;
    }

    char & operator[](const int index)
    {
        assert(index < sizeof(data_));
        return data_[index];
    }

    size_t capacity() const
    {
        return sizeof(data_);
    }

    bool operator==(const FixedBuffer<length> & obj)
    {
        return memcmp(data_, obj.data_, length) == 0;
    }

private:
    char data_[length];
};


}

#endif