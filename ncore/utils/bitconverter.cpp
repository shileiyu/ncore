#include "bitconverter.h"

namespace ncore
{


inline static char b2a(char ch)
{
    return 0 <= ch && ch <= 9 ? ch += 0x30 : ch += 0x57;
}

int BitConverter::BinToHexStr(const void * blob, size_t blob_size,
                              char * str, size_t count)
{
    auto buffer = reinterpret_cast<const uint8_t *>(blob);
    char ch = 0;
    auto begin = str;
    auto end = begin + count;

    if(buffer == 0 || blob_size == 0 || begin == 0 || count == 0)
        return 0;

    for(size_t i = 0 ; i < blob_size && str + 2 <= end; ++i)
    {
        ch = (buffer[i] >> 4) & 0xF;
        *str++ = b2a(ch);

        ch = buffer[i] & 0xF;
        *str++ = b2a(ch);
    }

    if(str < end)
        *str = 0;

    return str - begin;
}


int BitConverter::HexStrToBin(const char * str, size_t count,
                              void * blob, size_t blob_size)
{
    size_t amount = (count / 2) + (count % 2);

    if(blob == 0 || blob_size < amount)
        return 0;
    
    uint8_t * pret_real_buffer = reinterpret_cast<uint8_t *>(blob);

    for(size_t i = 0; i < count; i = i + 2)
    {
        char temp;
        char ch;
        //string 长度是奇数时 
        if((i == 0) && (count % 2))
        {
            ch = str[i];
            if('0' <= ch && ch <= '9')
                ch -= 0x30;
            else if('a' <= ch && ch <= 'f')
                ch -= 0x57;
            else if('A' <= ch && ch <= 'F')
                ch -= 0x37;
            else
                return 0; //not a hex digit
        
            *pret_real_buffer++ = static_cast<uint8_t>(ch);       
            --i;
            continue;
        }
        //处理长度是偶数位
        ch  = str[i];
        if('0' <= ch && ch <= '9')
            ch -= 0x30;
        else if('a' <= ch && ch <= 'f')
            ch -= 0x57;
        else if('A' <= ch && ch <= 'F')
            ch -= 0x37;
        else
            return 0;   //not a hex digit

        temp = ch << 4;

        ch = str[i+1];
        if('0' <= ch && ch <= '9')
            ch -= 0x30;
        else if('a' <= ch && ch <= 'f')
            ch -= 0x57;
        else if('A' <= ch && ch <= 'F')
            ch -= 0x37;
        else
            return 0;   //not a hex digit

        temp += ch;
        //存入到pret_buffer中去
        *pret_real_buffer++ = static_cast<uint8_t>(temp);
    }

    return amount;
}

uint16_t BitConverter::HexToChar(uint8_t hex)
{
    uint8_t lo = 0;
    uint8_t hi = 0;
    lo = hex & 0xF;
    hi = (hex >> 4) & 0xF;

    lo += (0 <= lo && lo <= 9)?0x30:0x37;
    hi +=(0 <= hi && hi <= 9)?0x30:0x37;

    return (lo << 8) | hi;
}


}
