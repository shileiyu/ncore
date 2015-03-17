#include "gtest\gtest.h"
#include <ncore/algorithm/hash.h>
#include <ncore/algorithm/crc.h>
#include <ncore/algorithm/md5.h>
#include <ncore/algorithm/sha1.h>

using namespace ncore;

TEST(HashTest, CRC16)
{
    struct
    {
        size_t size;
        char * data;
        uint16_t crc16;
    } gm[] = 
    {
        {
            0, 0,
            0,
        },
        {
            1, "\0",
            0
        },
        {
            1, "\xff",
            0x4040,
        },
        {   
            3, "foo",
            0xc38d,
        },
        {
            6, "foobar",
            0xb0c8,
        }
    };

    CRC16Provider crc16;
    Hash hash;
    uint16_t crc = 0xffff;
    for(size_t i = 0 ; i < sizeof(gm) / sizeof(gm[0]); ++i)
    {
        crc16.Update(gm[i].data, gm[i].size);
        crc16.Final(hash);
        crc16.Final(crc);

        EXPECT_EQ(2, hash.size());
        EXPECT_EQ(true, gm[i].crc16 == *(uint16_t*)hash.data());
        EXPECT_EQ(true, gm[i].crc16 == crc);

        crc16.Reset();
    }
}

TEST(HashTest, CRC32)
{
    struct
    {
        size_t size;
        char * data;
        uint32_t crc32;
    } gm[] = 
    {
        {
            0, 0,
            0,
        },
        {
            1, "\0",
            0xd202ef8d
        },
        {
            1, "\xff",
            0xff000000,
        },
        {   
            3, "foo",
            0x8c736521,
        },
        {
            6, "foobar",
            0x9ef61f95,
        }
    };

    CRC32Provider crc32;
    Hash hash;
    uint32_t crc = 0xffff;
    for(size_t i = 0 ; i < sizeof(gm) / sizeof(gm[0]); ++i)
    {
        crc32.Update(gm[i].data, gm[i].size);
        crc32.Final(hash);
        crc32.Final(crc);

        EXPECT_EQ(4, hash.size());
        EXPECT_EQ(true, gm[i].crc32 == *(uint32_t*)hash.data());
        EXPECT_EQ(true, gm[i].crc32 == crc);

        crc32.Reset();
    }
}

TEST(HashTest, Md5)
{
    struct
    {
        size_t size;
        char * data;
        char * md5;
    } gm[] = 
    {
        {
            0, "",
            "\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e",
        },
        {
            1, "a",
            "\x0c\xc1\x75\xb9\xc0\xf1\xb6\xa8\x31\xc3\x99\xe2\x69\x77\x26\x61",
        },
        {
            3, "abc",
            "\x90\x01\x50\x98\x3c\xd2\x4f\xb0\xd6\x96\x3f\x7d\x28\xe1\x7f\x72",
        },
        {
            14, "message digest",
            "\xf9\x6b\x69\x7d\x7c\xb7\x93\x8d\x52\x5a\x2f\x31\xaa\xf1\x61\xd0",
        },
        {
            26, "abcdefghijklmnopqrstuvwxyz",
            "\xc3\xfc\xd3\xd7\x61\x92\xe4\x00\x7d\xfb\x49\x6c\xca\x67\xe1\x3b",
        },
        {
            62, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
            "\xd1\x74\xab\x98\xd2\x77\xd9\xf5\xa5\x61\x1c\x2c\x9f\x41\x9d\x9f",
        },
    };

    MD5Provider md5;
    Hash hash;
    for(size_t i = 0 ; i < sizeof(gm) / sizeof(gm[0]); ++i)
    {
        md5.Update(gm[i].data, gm[i].size);
        md5.Final(hash);

        EXPECT_EQ(16, hash.size());
        EXPECT_EQ(0, memcmp(hash.data(), gm[i].md5, 16));
    
        md5.Reset();
    }
}

TEST(HashTest, SHA1)
{
    struct
    {
        size_t repeat;
        size_t size;
        char * data;
        char * sha1;
    } gm[] = 
    {
        {
            0, 0, 0,
            "\xDA\x39\xA3\xEE\x5E\x6B\x4B\x0D\x32\x55"
            "\xBF\xEF\x95\x60\x18\x90\xAF\xD8\x07\x09",
        },
        {
            1, 3, "abc",
            "\xa9\x99\x3e\x36\x47\x06\x81\x6a\xba\x3e"
            "\x25\x71\x78\x50\xc2\x6c\x9c\xd0\xd8\x9d",
        },
        {
            1, 56, "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "\x84\x98\x3E\x44\x1C\x3B\xD2\x6E\xBA\xAE"
            "\x4A\xA1\xF9\x51\x29\xE5\xE5\x46\x70\xF1",
        },
        {
            1000000, 1, "a",
            "\x34\xaa\x97\x3c\xd4\xc4\xda\xa4\xf6\x1e"
            "\xeb\x2b\xdb\xad\x27\x31\x65\x34\x01\x6f",
        },
        {
            10, 64, "0123456701234567012345670123456701234567012345670123456701234567",
            "\xde\xa3\x56\xa2\xcd\xdd\x90\xc7\xa7\xec"
            "\xed\xc5\xeb\xb5\x63\x93\x4f\x46\x04\x52",
        },
    };

    SHA1Provider sha1;
    Hash hash;
    for(size_t i = 0 ; i < sizeof(gm) / sizeof(gm[0]); ++i)
    {
        for(size_t j = 0; j < gm[i].repeat; ++j)
        {
            sha1.Update(gm[i].data, gm[i].size);
        }
        
        sha1.Final(hash);

        EXPECT_EQ(20, hash.size());
        EXPECT_EQ(0, memcmp(hash.data(), gm[i].sha1, 20));
    
        sha1.Reset();
    }
}

