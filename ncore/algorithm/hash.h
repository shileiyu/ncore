#ifndef NCORE_ALGORITHM_HASH_H_
#define NCORE_ALGORITHM_HASH_H_

#include <ncore/ncore.h>

namespace ncore
{


class Hash
{
public:
    Hash();
    Hash(const void * data, size_t size);
    Hash(const Hash & obj);

    ~Hash();

    Hash & operator=(const Hash & obj);
    bool operator==(const Hash & right);
    bool operator!=(const Hash & right);

    bool Compare(const char * hex);

    std::string ToString();

    const uint8_t * data() const;
    size_t size() const;
private:
    size_t size_;
    uint8_t data_[64];
};

class HashProvider
{
public:
    /*重置*/
    virtual void Reset() = 0;

    /*提交数据*/
    virtual void Update(const void * blob, size_t size) = 0;

    /*取Hash*/
    virtual void Final(Hash & hash) = 0;
};


}

#endif
