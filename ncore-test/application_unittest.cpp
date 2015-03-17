#include <gtest\gtest.h>
#include <ncore/sys/application.h>


TEST(ApplicationTest, ArgParser)
{
    using namespace ncore;

    std::string cmd_line; 
    Options result;
    //测试正常参数
    cmd_line = "-mode = test1 -user -name-io=1234";
    result = ParseOptions(cmd_line);
    ASSERT_FALSE(result.empty());
    ASSERT_EQ(4, result.size());
    ASSERT_STREQ(result["mode"].data(), "test1");
    ASSERT_STREQ(result["io"].data(), "1234");
    ASSERT_EQ(true, result.HasOption("user"));
    ASSERT_EQ(true, result.HasOption("name"));

    cmd_line = "\"J:\\code \\platinum\\trunk\\nbase\\sys\" -mode=test";
    result = ParseOptions(cmd_line);
    ASSERT_FALSE(result.empty());
    ASSERT_STREQ(result["mode"].data(), "test");

    cmd_line = "\"J:\\code \\platinum\\trunk\\nbase\\sys\" -mode\t\f=test";
    result = ParseOptions(cmd_line);
    ASSERT_FALSE(result.empty());
    ASSERT_STREQ(result["mode"].data(), "test");

    cmd_line = "J:\\code\\platinum\\trunk\\nbase\\sys -mode\t\f=\"testxxx\"";
    result = ParseOptions(cmd_line);
    ASSERT_FALSE(result.empty());
    ASSERT_STREQ(result["mode"].data(), "testxxx");

    //测试"不匹配时 options应该为空
    cmd_line = "-mode\t\f=\"testxxx";
    result = ParseOptions(cmd_line);
    ASSERT_TRUE(result.empty());
    //测试以空白字符结尾 options应该为空
    cmd_line = "-mode\t\f=    \t";
    result = ParseOptions(cmd_line);
    ASSERT_TRUE(result.empty());
    cmd_line = "-mode\t\f=    \t-name= user";
    result = ParseOptions(cmd_line);
    ASSERT_TRUE(result.empty());

    //测试多组参数存在
    cmd_line = "  \"mmmm yyy\"   -mode\t\f= testxxx   -out   aa-name=user bb -io=1234 ";
    result = ParseOptions(cmd_line);
    ASSERT_FALSE(result.empty());
    ASSERT_EQ(4, result.size());
    ASSERT_STREQ(result["mode"].data(), "testxxx");
    ASSERT_STREQ(result["name"].data(), "user");
    ASSERT_STREQ(result["io"].data(), "1234");
    ASSERT_TRUE(result.HasOption("out"));
}
