#include <gtest\gtest.h>
#include <ncore/base/exception.h>

using namespace ncore;

class TestException : public Exception
{
public:
    TestException()
        : Exception("This is a test exception")
    {
    }
};

TEST(ExceptionTest, Generic)
{
    int path = 0;
    try
    {
        throw TestException();
    }
    catch(TestException e)
    {
        EXPECT_EQ(0, strcmp(e.message(), "This is a test exception"));
        SUCCEED() << "Exception type match";
    }
    catch(Exception e)
    {
        FAIL() << "Exception type doesn't match";
    }

    try
    {
        throw TestException();
    }
    catch(Exception e)
    {
        SUCCEED() << "Exception type match";
    }
}

