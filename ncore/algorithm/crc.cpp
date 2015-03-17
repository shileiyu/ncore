#include "crc.h"

namespace ncore
{


/*CRC16*/
CRC16Provider::CRC16Provider()
{
    Reset();
}

void CRC16Provider::Reset()
{
    crc_ = 0;
}


void CRC16Provider::Update(const void * data, size_t size)
{
    static bool crc_tab16_init = false;
    static uint16_t crc_tab16[256];

    if(crc_tab16_init == false)
    {
        init_crc16_tab(crc_tab16);
        crc_tab16_init = true;
    }

    uint16_t tmp = 0;
    uint16_t short_c = 0;
    auto buffer = reinterpret_cast<const uint8_t *>(data);

    if(size == 0 || buffer == 0)
        return;
    
    while(size--)
    {
        short_c = 0x00ff & static_cast<uint16_t>(*buffer++);
        tmp =  crc_^ short_c;
        crc_ = (crc_ >> 8) ^ crc_tab16[ tmp & 0xff ];
    }

    return;
}

void CRC16Provider::Final(Hash & hash)
{
    hash = Hash(&crc_, sizeof(crc_));
}

void CRC16Provider::Final(uint16_t & crc)
{
    crc = crc_;
}

void CRC16Provider::init_crc16_tab(uint16_t * crc_tab16) 
{
    int i, j;
    uint16_t crc, c;

    for (i = 0; i < 256; ++i) 
    {
        crc = 0;
        c   = static_cast<uint16_t>(i);
        for (j = 0; j < 8; ++j)
        {
            if ( (crc ^ c) & 0x0001 )
                crc = ( crc >> 1 ) ^ 0xA001;
            else
                crc =   crc >> 1;

            c = c >> 1;
        }
        crc_tab16[i] = crc;
    }
}

/*CRC32*/


CRC32Provider::CRC32Provider()
{
    Reset();
}

void CRC32Provider::Reset()
{
    crc_ = 0xffffffff;
}

void CRC32Provider::Update(const void * data, size_t size)
{
    static bool crc_tab32_init = false;
    static uint32_t crc_tab32[256];

    if(crc_tab32_init == false)
    {
        init_crc32_tab(crc_tab32);
        crc_tab32_init = true;
    }

    uint32_t tmp = 0;
    uint32_t long_c = 0;
    auto buffer = reinterpret_cast<const uint8_t *>(data);

    if(size == 0 || buffer == 0)
        return;

    while(size--)
    {
        long_c = 0x000000ff & static_cast<uint32_t>(*buffer++);
        tmp = crc_ ^ long_c;
        crc_ = (crc_ >> 8) ^ crc_tab32[ tmp & 0xff ];
    }

    return;
}

void CRC32Provider::Final(Hash & hash)
{
    uint32_t crc = crc_ ^ 0xffffffff;
    hash = Hash(&crc, sizeof(crc));
}

void CRC32Provider::Final(uint32_t & crc)
{
    crc = crc_ ^ 0xffffffff;
}

void CRC32Provider::init_crc32_tab(uint32_t * crc_tab32)
{
    int i, j;
    uint32_t crc;

    for (i=0; i < 256; i++) 
    {
        crc = static_cast<uint32_t>(i);

        for (j=0; j < 8; j++) 
        {

            if ( crc & 0x00000001) 
                crc = ( crc >> 1 ) ^ 0xEDB88320;
            else
                crc =   crc >> 1;
        }
        crc_tab32[i] = crc;
    }
}


}