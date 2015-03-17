#include <gtest\gtest.h>
#include <ncore/sys/timer.h>
#include <ncore/utils/handy.h>

using namespace ncore;

class TimerOnTimeTest
{
public:
    TimerOnTimeTest()
        : inter_time_(0)
        , begin_time_(0)
        , error_time_(0)
        , is_completed_(false)
    {
        ap_.Register(this, &TimerOnTimeTest::OnTime);
    }

    ~TimerOnTimeTest()
    {}

    bool init()
    {
        bool succeed = false;
        succeed = timer_.init(ap_);
        return succeed;
    }

    void fini()
    {
        timer_.fini();
    }

    uint32_t error_time() const
    {
        return error_time_;
    }

    bool is_completed() const
    {
        return is_completed_;
    }

    void Run()
    {
        SleepEx(0, true);
    }

    bool SetTime(uint32_t ms)
    {
        bool succeed = false;
        is_completed_ = false;
        inter_time_ = ms;
        timer_.SetInterval(inter_time_);
        succeed = timer_.Start();
        if (!succeed)
            return false;
        begin_time_ = GetTickCount();
        error_time_ = 0;
        return true;
    }

private:
    void OnTime()
    {
        const int32_t elapsed_time = GetTickCount() - begin_time_;
        const int32_t error_time = elapsed_time - inter_time_;
        error_time_ = error_time < 0 ? -error_time : error_time;
        is_completed_ = true;
        timer_.Stop();
    }

private:
    Timer timer_;
    TimerProcAdapter<TimerOnTimeTest> ap_;
    uint32_t inter_time_;
    unsigned long begin_time_;
    uint32_t error_time_;
    bool is_completed_;
};

class TimerTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
    }
};

TEST_F(TimerTest, OnTime)
{
    bool succeed = false;

    TimerOnTimeTest timer_ontime_test;

    succeed = timer_ontime_test.init();
    ASSERT_TRUE(succeed);

    uint32_t times[] = {1000, 1200, 2100};

    for (size_t count = 0; count < countof(times); ++count)
    {
        const uint32_t time_ms = times[count];

        succeed = timer_ontime_test.SetTime(time_ms);
        if (!succeed) break;

        while (!timer_ontime_test.is_completed())
            timer_ontime_test.Run();

        const uint32_t error_time = timer_ontime_test.error_time();
        EXPECT_TRUE(error_time <= 50);
    }

    timer_ontime_test.fini();
}
