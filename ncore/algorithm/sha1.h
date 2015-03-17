#ifndef NCORE_ALGORITHM_SHA1_H_
#define NCORE_ALGORITHM_SHA1_H_

#include "hash.h"

namespace ncore
{


class SHA1Provider : public HashProvider
{
public:
    static const int kHashSize = 20;
private:
    struct SHA1Context
    {
        uint32_t Message_Digest[5]; /* Message Digest (output)          */
        uint32_t Length_Low;        /* Message length in bits           */
        uint32_t Length_High;       /* Message length in bits           */
        uint8_t Message_Block[64];  /* 512-bit message blocks      */
        int Message_Block_Index;    /* Index into message block array   */
        int Computed;               /* Is the digest computed?          */
        int Corrupted;              /* Is the message digest corruped?  */
    };
public:
    SHA1Provider();
    void Reset() override;
    void Update(const void * data, size_t size) override;
    void Final(Hash & hash) override;
private:
    void ProcessMessageBlock();
    void PadMessage();
private:
    SHA1Context context_;
};


}

#endif