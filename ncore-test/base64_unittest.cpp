#include <gtest/gtest.h>
#include <ncore/encoding/base64.h>



TEST(Base64Test, GoldMaster)
{
    using namespace ncore;

    typedef std::map<std::string, std::string> StringTable;

    StringTable gm;
    gm[""] = "";
    gm["f"] = "Zg==";
    gm["fo"] = "Zm8=";
    gm["foo"] = "Zm9v";
    gm["foob"] = "Zm9vYg==";
    gm["fooba"] = "Zm9vYmE=";
    gm["foobar"] = "Zm9vYmFy";

    for(auto iter = gm.begin(); iter != gm.end(); ++iter)
    {
        char code[32];
        char blob[32];
        auto & key = iter->first;
        auto & value = iter->second;
        int ret = -1;

        ret = Base64::Encode(key.data(), key.length(), code, 32);
        EXPECT_EQ(value.length(), ret);
        EXPECT_EQ(0, value.compare(code));

        ret = Base64::Decode(code, blob, 32);
        EXPECT_EQ(key.length(), ret);
        EXPECT_EQ(0, key.compare(blob));
    }
}

