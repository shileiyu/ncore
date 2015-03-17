#include <ncore/utils/bitconverter.h>
#include "hash.h"

namespace ncore
{


static const size_t kMaxHashSize = 64;

Hash::Hash()
    : size_(0)
{
    memset(data_, 0, sizeof(data_));
}

Hash::Hash(const void * data, size_t size)
    : size_(size)
{
    if(size > sizeof(data_))
        return;

    memcpy(data_, data, size_);
}

Hash::Hash(const Hash & obj)
    : size_(obj.size_)
{
    memcpy(data_, obj.data_, size_);
}

Hash::~Hash()
{
}

Hash & Hash::operator=(const Hash & obj)
{
    size_ = obj.size_;
    memcpy(data_, obj.data_, size_);
    return *this;
}

bool Hash::operator==(const Hash & right)
{
    if(size_ != right.size_)
        return false;

    return memcmp(data_, right.data_, size_) == 0;
}

bool Hash::operator!=(const Hash & right)
{
    if(size_ == right.size_)
        return false;

    return memcmp(data_, right.data_, size_) != 0;
}

bool Hash::Compare(const char * string)
{
    if(string == nullptr)
        return false;
    size_t length = strlen(string);
    if(length != size_ * 2)
        return false;

    uint8_t comparend[kMaxHashSize] = {0};
    BitConverter::HexStrToBin(string, length, comparend, kMaxHashSize);
    return memcmp(data_, comparend, size_) == 0;
}

std::string Hash::ToString()
{
    char string[kMaxHashSize] = {0};
    BitConverter::BinToHexStr(data_, size_, string, kMaxHashSize);
    return string;
}

const uint8_t * Hash::data() const
{
    return data_;
}

size_t Hash::size() const
{
    return size_;
}


}
