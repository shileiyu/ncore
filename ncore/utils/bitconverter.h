#ifndef NCORE_UTILS_BITCONVERTER_H_
#define NCORE_UTILS_BITCONVERTER_H_

#include <ncore/ncore.h>

namespace ncore
{

  
class BitConverter
{
public:
    /*
    convert buffer content into ascii.
    exp.
    { 0x12,0x34,0x56,0x78 } -> "12345678"
    */
    static int BinToHexStr(const void * blob, size_t blob_size,
                           char * hexstr, size_t count);

    /*
    convert a string into hex
    exp.
    "12345678" -> { 0x12,0x34,0x56,0x78 }
    */
    static int HexStrToBin(const char * hexstr, size_t count,
                           void * blob, size_t blob_size);

    static uint16_t HexToChar(uint8_t hex);
};

typedef BitConverter BitCnvrt;


}
#endif
