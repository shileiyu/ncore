#include <gtest\gtest.h>
#include <ncore/utils/bitconverter.h>


TEST(BitConverterTest, BinToHexStr)
{
    using namespace ncore;

    uint8_t bin1[] = { 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef};
    uint8_t bin2[] = { 0x1, 0x2, 0x3, 0x4};

    int ret = 0;
    char hexstr[20];
    ret = BitConverter::BinToHexStr(bin1, sizeof(bin1), hexstr, 20);
    EXPECT_EQ(16, ret);
    EXPECT_EQ(0, _stricmp(hexstr, "1234567890abcdef"));
    ret = BitConverter::BinToHexStr(bin2, sizeof(bin2), hexstr, 20);
    EXPECT_EQ(8, ret);
    EXPECT_EQ(0, _stricmp(hexstr, "01020304"));
    ret = BitConverter::BinToHexStr(bin1, sizeof(bin1), hexstr, 0);
    EXPECT_EQ(0, ret);
    ret = BitConverter::BinToHexStr(bin1, sizeof(bin1), hexstr, 1);
    EXPECT_EQ(0, ret);
    ret = BitConverter::BinToHexStr(bin1, sizeof(bin1), hexstr, 2);
    EXPECT_EQ(2, ret);
}

TEST(BitConverterTest, HexStrToBin)
{
    using namespace ncore;

    uint8_t bin1[] = { 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef};
    uint8_t bin2[] = { 0xa, 0xbc, 0xde};
    char hex1[] = {"1234567890abcdef"};
    char hex2[] = {"abcde"};

    int ret = 0;
    uint8_t bin[20];
    ret = BitConverter::HexStrToBin(hex1, strlen(hex1), bin, sizeof(bin));
    EXPECT_EQ(8, ret);
    EXPECT_EQ(0, memcmp(bin, bin1, sizeof(bin1)));
    ret = BitConverter::HexStrToBin(hex2, strlen(hex2), bin, sizeof(bin));
    EXPECT_EQ(3, ret);
    EXPECT_EQ(0, memcmp(bin, bin2, sizeof(bin2)));
}

