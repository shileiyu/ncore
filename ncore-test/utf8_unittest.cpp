#include <gtest\gtest.h>
#include <ncore/encoding/utf8.h>
#include <ncore/utils/karma.h>

struct Pair
{
    wchar_t * unicode;
    char * utf8;
};

TEST(UTF8Test, GoldMaster)
{
    using namespace ncore;

    Pair gm[] = 
    {
        {
            L"english",           
            "english"
        },
        {
            L"Русский текст",     
            "\xD0\xA0\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xB8\xD0\xB9\x20\xD1"
            "\x82\xD0\xB5\xD0\xBA\xD1\x81\xD1\x82"
        },
        {
            L"日本語のテキスト",   
            "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E\xE3\x81\xAE\xE3\x83\x86\xE3"
            "\x82\xAD\xE3\x82\xB9\xE3\x83\x88"
        },
        {
            L"中文简体",
            "\xE4\xB8\xAD\xE6\x96\x87\xE7\xAE\x80\xE4\xBD\x93"
        },
    };
    
    wchar_t unicode[32];
    char utf8[32];
    int result = 0;

    for(int i = 0; i < sizeof(gm)/sizeof(gm[0]); ++i)
    {
        
        result = UTF8::Encode(gm[i].unicode, wcslen(gm[i].unicode), utf8, 32);
        utf8[result] = 0;
        EXPECT_EQ(strlen(gm[i].utf8), result);
        EXPECT_EQ(0, strcmp(utf8, gm[i].utf8));
        result = UTF8::Decode(gm[i].utf8, strlen(gm[i].utf8), unicode, 32);
        unicode[result] = 0;
        EXPECT_EQ(wcslen(gm[i].unicode), result);
        EXPECT_EQ(0, wcscmp(unicode, gm[i].unicode));
    }

    result = UTF8::Encode(L"", 1, utf8, 1);
    EXPECT_EQ(1, result);
    result = UTF8::Decode("", 1, unicode, 1);
    EXPECT_EQ(1, result);
}

TEST(KarmaTest, UTF8)
{
    using namespace ncore;

    auto wide = Karma::FromUTF8("简体中文");
    auto utf8 = Karma::ToUTF8(L"简体中文");
    EXPECT_EQ(4, wide.length());
    EXPECT_STREQ(L"简体中文", wide.data());
    EXPECT_EQ(12, utf8.length());
    EXPECT_STREQ("简体中文", utf8.data());
    auto explict = Karma::FromUTF8("\0简体中文", 13);
}
