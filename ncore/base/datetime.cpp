#include "datetime.h"

namespace ncore
{


const int64_t DateTime::kInvalidTick = -1;
const int64_t DateTime::kWindowsEpochDeltaSeconds = 11644473600;
const int64_t DateTime::kMillisecondsPerSecond = 1000;
const int64_t DateTime::kMicrosecondsPerSecond = 1000000;


Timezone::Timezone()
{
    bias_ = 0;
    abbr_ = "GMT";
    name_ = "Greenwich Mean Time";
}

DateTime DateTime::UTCNow()
{
    /*
      FILETIME represents ticks in 100 nanoseconds
      we translate it into milliseconds.
    */
    FILETIME ft;
    ::GetSystemTimeAsFileTime(&ft);
    int64_t & tick = reinterpret_cast<int64_t &>(ft);
    tick /= 10;
    return DateTime(tick);
}

DateTime DateTime::FromUnixTime(time_t unix_time)
{
    int64_t tick = unix_time;
    tick += kWindowsEpochDeltaSeconds;
    tick *=  kMicrosecondsPerSecond;
    return DateTime(tick);
}

DateTime::DateTime()
    : tick_(-1)
{
}

DateTime::DateTime(int64_t tick)
    : tick_(tick)
{
}

DateTime::~DateTime()
{
}

bool DateTime::IsValid() const
{
    return tick_ != kInvalidTick;
}

void DateTime::set_tick(int64_t tick)
{
    tick_ = tick;
}

int64_t DateTime::tick() const
{
    return tick_;
}

size_t DateTime::Format(char * str, size_t size, const char * fmt) const
{
    if(!IsValid())
        return 0;
    time_t unix_time = ToUnixTime();
    tm human_time = { 0 };
    if (gmtime_s(&human_time, &unix_time))
        return 0;
    return strftime(str, size, fmt, &human_time);
}

size_t DateTime::Format(wchar_t * str, size_t size, const wchar_t * fmt) const
{
    if (!IsValid())
        return 0;
    time_t unix_time = ToUnixTime();
    tm human_time = { 0 };
    if (gmtime_s(&human_time, &unix_time))
        return 0;
    return wcsftime(str, size, fmt, &human_time);
}

time_t DateTime::ToUnixTime() const
{
    if(!IsValid())
        return kInvalidTick;
    time_t tick = tick_;
    tick -= kWindowsEpochDeltaSeconds * kMicrosecondsPerSecond;
    tick /= kMicrosecondsPerSecond; 
    return tick;
}

bool DateTime::operator == (const DateTime & right) const
{
    return tick_ == right.tick_;
}

bool DateTime::operator != (const DateTime & right) const
{
    return tick_ != right.tick_;
}

TimeSpan DateTime::operator - (const DateTime & right) const
{
    return std::abs(tick_ - right.tick_);
}


}