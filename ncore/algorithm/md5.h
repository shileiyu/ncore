#ifndef NCORE_ALGORITHM_MD5_H_
#define NCORE_ALGORITHM_MD5_H_

#include "hash.h"

namespace ncore
{


class MD5Provider : public HashProvider
{
public:
    static const int kHashSize = 16;
private:
    struct Md5Context
    {
        uint32_t i[2];          /* number of _bits_ handled mod 2^64 */
        uint32_t buf[4];        /* scratch buffer */
        uint8_t in[64];         /* input buffer */
        uint8_t digest[kHashSize];     /* actual digest after MD5Final call */
    };
public:
    MD5Provider();
    void Reset() override;
    void Update(const void * data, size_t size) override;
    void Final(Hash & hash) override;
private:
    void Transform(uint32_t * buf, uint32_t * in);
private:
    Md5Context context_;
};


}

#endif