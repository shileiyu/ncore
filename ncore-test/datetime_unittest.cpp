#include <gtest\gtest.h>
#include <ncore/base/datetime.h>

using namespace ncore;


TEST(DateTimeTest, Generic)
{
    time_t tm_now;
    time(&tm_now);
    DateTime dt1 = DateTime::UTCNow();
    DateTime dt2(dt1);
    DateTime dt3 = dt2;
    EXPECT_EQ(true, dt1.ToUnixTime() == tm_now);
    EXPECT_EQ(true, dt2 == dt1); 
    EXPECT_EQ(true, dt3 == dt2);

    DateTime dt4;
    DateTime dt5;
    dt4.set_tick(1);
    dt5.set_tick(2);
    EXPECT_EQ(true, dt4 != dt5);
}

