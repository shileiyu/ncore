#include "gtest\gtest.h"
#include <ncore/base/datetime.h>
#include <ncore/base/timespan.h>

using namespace ncore;

TEST(TimeSpanTest, Generic)
{
    TimeSpan ts(0);
    EXPECT_TRUE(0 == ts.tick());
    EXPECT_TRUE(0 == ts.Days());
    EXPECT_TRUE(0 == ts.Hours());
    EXPECT_TRUE(0 == ts.Minutes());
    EXPECT_TRUE(0 == ts.Seconds());
    EXPECT_TRUE(0 == ts.Milliseconds());

    ts = 937840050000;  // 10 days, 20 hours, 30 minutes, 40 seconds, 50 milliseconds
    EXPECT_TRUE(10 == ts.Days());
    EXPECT_TRUE(20 == ts.Hours());
    EXPECT_TRUE(30 == ts.Minutes());
    EXPECT_TRUE(40 == ts.Seconds());
    EXPECT_TRUE(50 == ts.Milliseconds());

    ts.set_tick(11122233344000);  // 128 days, 17 hours, 30 minutes, 33 seconds, 344 milliseconds
    EXPECT_TRUE(128 == ts.Days());
    EXPECT_TRUE(17 == ts.Hours());
    EXPECT_TRUE(30 == ts.Minutes());
    EXPECT_TRUE(33 == ts.Seconds());
    EXPECT_TRUE(344 == ts.Milliseconds());

    ts.set_tick(11122233344000 - 937840050000);
    EXPECT_TRUE(117 == ts.Days());
    EXPECT_TRUE(20 == ts.Hours());
    EXPECT_TRUE(59 == ts.Minutes());
    EXPECT_TRUE(53 == ts.Seconds());
    EXPECT_TRUE(294 == ts.Milliseconds());
}

TEST(TimeSpanTest, Operator)
{
    TimeSpan ts1(1);
    TimeSpan ts2(10);
    ts2 += ts1;
    EXPECT_TRUE(11 == ts2.tick());
    ts2 -= ts1;
    EXPECT_TRUE(10 == ts2.tick());

    EXPECT_TRUE(ts1 != ts2);
    EXPECT_TRUE(ts1 < ts2);
    EXPECT_TRUE(ts1 <= ts2);
    EXPECT_TRUE(ts2 > ts1);
    EXPECT_TRUE(ts2 >= ts1);

    TimeSpan ts3(0);
    ts3 = ts1 + ts2;
    EXPECT_TRUE(11 == ts3.tick());
    ts3 = ts1 - ts2;
    EXPECT_TRUE(-9 == ts3.tick());

    TimeSpan ts4(-9);
    EXPECT_TRUE(ts3 == ts4);
    EXPECT_TRUE(ts1 < ts3);
    EXPECT_TRUE(ts2 > ts3);
}

TEST(TimeSpanTest, From)
{
    TimeSpan ts1 = TimeSpan::FromMilliseconds(1);
    EXPECT_TRUE(1 == ts1.TotalMilliseconds());
    EXPECT_TRUE(1 == ts1.Milliseconds());

    ts1 = TimeSpan::FromMilliseconds(1000);
    EXPECT_TRUE(1000 == ts1.TotalMilliseconds());
    EXPECT_TRUE(0 == ts1.Milliseconds());
    EXPECT_TRUE(1 == ts1.Seconds());

    ts1 = TimeSpan::FromDays(1);
    EXPECT_TRUE(0 == ts1.Milliseconds());
    EXPECT_TRUE(0 == ts1.Seconds());
    EXPECT_TRUE(0 == ts1.Minutes());
    EXPECT_TRUE(0 == ts1.Hours());
    EXPECT_TRUE(1 == ts1.Days());

    TimeSpan ts2 = TimeSpan::FromTotalHours(24.0);
    EXPECT_TRUE(ts1 == ts2);
}

