#include <gtest/gtest.h>
#include <ncore/utils/period.h>

using namespace ncore;

class PeriodTestHost
{
public:
    PeriodTestHost()
    {
        ticks_.reserve(10);
        tick_logger_.Register(this, &PeriodTestHost::LogTick);
        tick_logger_.set_internal(1000);
    }

    void LogTick()
    {
        ticks_.push_back(GetTickCount());
    }

    void Run()
    {
        tick_logger_.Run();
    }


    std::vector<DWORD> ticks_;
    Period<PeriodTestHost> tick_logger_;

};

TEST(Period, Simple)
{
    PeriodTestHost host;

    int count = 1000;
    while(--count)
    {
        host.Run();
        Sleep(10);
    }

    count = host.ticks_.size() - 1;
    for(auto i = 0; i < count; ++i)
    {
        int diff = host.ticks_[i + 1] - host.ticks_[i] - 1000;
        EXPECT_LT(std::abs(diff), 50);
    }

}

