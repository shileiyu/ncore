#include "sha1.h"

#if defined _MSC_VER
#define SHA1CircularShift(bits,word) \
    _rotl(word, bits)
#else
#define SHA1CircularShift(bits,word) \
    ((((word) << (bits)) & 0xFFFFFFFF) | ((word) >> (32-(bits))))
#endif

namespace ncore
{


SHA1Provider::SHA1Provider()
{
    Reset();
}

void SHA1Provider::Reset()
{
    context_.Length_Low = 0;
    context_.Length_High = 0;
    context_.Message_Block_Index = 0;

    context_.Message_Digest[0] = 0x67452301;
    context_.Message_Digest[1] = 0xEFCDAB89;
    context_.Message_Digest[2] = 0x98BADCFE;
    context_.Message_Digest[3] = 0x10325476;
    context_.Message_Digest[4] = 0xC3D2E1F0;

    context_.Computed = 0;
    context_.Corrupted = 0;
}

void SHA1Provider::Update(const void * data, size_t size)
{
    if (context_.Computed || context_.Corrupted)
    {
        context_.Corrupted = 1;
        return;
    }

    if (size == 0 || data == 0)
        return;

    auto buffer = reinterpret_cast<const uint8_t *>(data);

    while(size-- && !context_.Corrupted)
    {
        context_.Message_Block[context_.Message_Block_Index++] = *buffer++;

        context_.Length_Low += 8;
        /* Force it to 32 bits */
        context_.Length_Low &= 0xFFFFFFFF;
        if (context_.Length_Low == 0)
        {
            context_.Length_High++;
            /* Force it to 32 bits */
            context_.Length_High &= 0xFFFFFFFF;
            if (context_.Length_High == 0)
            {
                /* Message is too long */
                context_.Corrupted = 1;
            }
        }

        if (context_.Message_Block_Index == 64)
        {
            ProcessMessageBlock();
        }
    }
}

void SHA1Provider::Final(Hash & hash)
{
    uint8_t digest_swaped[20];
    uint8_t * digest = 0;

    if (context_.Corrupted)
        return;

    if (!context_.Computed)
    {
        PadMessage();
        context_.Computed = 1;
    }

    digest = reinterpret_cast<uint8_t *>(context_.Message_Digest);

    digest_swaped[0] =  digest[3];
    digest_swaped[1] =  digest[2];
    digest_swaped[2] =  digest[1];
    digest_swaped[3] =  digest[0];

    digest_swaped[4] =  digest[7];
    digest_swaped[5] =  digest[6];
    digest_swaped[6] =  digest[5];
    digest_swaped[7] =  digest[4];

    digest_swaped[8] =  digest[11];
    digest_swaped[9] =  digest[10];
    digest_swaped[10] =  digest[9];
    digest_swaped[11] =  digest[8];

    digest_swaped[12] =  digest[15];
    digest_swaped[13] =  digest[14];
    digest_swaped[14] =  digest[13];
    digest_swaped[15] =  digest[12];

    digest_swaped[16] =  digest[19];
    digest_swaped[17] =  digest[18];
    digest_swaped[18] =  digest[17];
    digest_swaped[19] =  digest[16];

    hash = Hash(digest_swaped, sizeof(digest_swaped));
}

void SHA1Provider::ProcessMessageBlock()
{
    static const uint32_t K[] =            /* Constants defined in SHA-1   */      
    {
        0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC,0xCA62C1D6
    };

    int         t;                  /* Loop counter                 */
    uint32_t    temp;               /* Temporary word value         */
    uint32_t    W[80];              /* Word sequence                */
    uint32_t    A, B, C, D, E;      /* Word buffers                 */

    /*
     *  Initialize the first 16 words in the array W
     */
    for(t = 0; t < 16; t++)
    {
        W[t] = (context_.Message_Block[t * 4]) << 24;
        W[t] |= (context_.Message_Block[t * 4 + 1]) << 16;
        W[t] |= (context_.Message_Block[t * 4 + 2]) << 8;
        W[t] |= (context_.Message_Block[t * 4 + 3]);
    }

    for(t = 16; t < 80; t++)
    {
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = context_.Message_Digest[0];
    B = context_.Message_Digest[1];
    C = context_.Message_Digest[2];
    D = context_.Message_Digest[3];
    E = context_.Message_Digest[4];

    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    context_.Message_Digest[0] = (context_.Message_Digest[0] + A) & 0xFFFFFFFF;
    context_.Message_Digest[1] = (context_.Message_Digest[1] + B) & 0xFFFFFFFF;
    context_.Message_Digest[2] = (context_.Message_Digest[2] + C) & 0xFFFFFFFF;
    context_.Message_Digest[3] = (context_.Message_Digest[3] + D) & 0xFFFFFFFF;
    context_.Message_Digest[4] = (context_.Message_Digest[4] + E) & 0xFFFFFFFF;

    context_.Message_Block_Index = 0;
}

void SHA1Provider::PadMessage()
{
    /*
     *  Check to see if the current message block is too small to hold
     *  the initial padding bits and length.  If so, we will pad the
     *  block, process it, and then continue padding into a second
     *  block.
     */
    if (context_.Message_Block_Index > 55)
    {
        context_.Message_Block[context_.Message_Block_Index++] = 0x80;
        while(context_.Message_Block_Index < 64)
        {
            context_.Message_Block[context_.Message_Block_Index++] = 0;
        }

        ProcessMessageBlock();

        while(context_.Message_Block_Index < 56)
        {
            context_.Message_Block[context_.Message_Block_Index++] = 0;
        }
    }
    else
    {
        context_.Message_Block[context_.Message_Block_Index++] = 0x80;
        while(context_.Message_Block_Index < 56)
        {
            context_.Message_Block[context_.Message_Block_Index++] = 0;
        }
    }

    /*
     *  Store the message length as the last 8 octets
     */
    context_.Message_Block[56] = (context_.Length_High >> 24) & 0xFF;
    context_.Message_Block[57] = (context_.Length_High >> 16) & 0xFF;
    context_.Message_Block[58] = (context_.Length_High >> 8) & 0xFF;
    context_.Message_Block[59] = (context_.Length_High) & 0xFF;
    context_.Message_Block[60] = (context_.Length_Low >> 24) & 0xFF;
    context_.Message_Block[61] = (context_.Length_Low >> 16) & 0xFF;
    context_.Message_Block[62] = (context_.Length_Low >> 8) & 0xFF;
    context_.Message_Block[63] = (context_.Length_Low) & 0xFF;

    ProcessMessageBlock();
}


}