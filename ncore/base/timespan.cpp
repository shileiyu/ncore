#include "timespan.h"

namespace ncore
{


const int32_t TimeSpan::kHoursPerDay = 24;
const int32_t TimeSpan::kMinutesPerHour = 60;
const int32_t TimeSpan::kSecondsPerMinute = 60;
const int32_t TimeSpan::kMillisecondsPerSecond = 1000;
const int32_t TimeSpan::kMicrosecondsPerMillisecond = 1000;
const int32_t TimeSpan::kMicrosecondsPerSecond = 1000000;
const int32_t TimeSpan::kMicrosecondsPerMinute = 60000000;
const int64_t TimeSpan::kMicrosecondsPerHour = 3600000000;
const int64_t TimeSpan::kMicrosecondsPerDay = 86400000000;

TimeSpan TimeSpan::FromTotalMilliseconds(double value)
{
    return TimeSpan(value * kMicrosecondsPerMillisecond);
}

TimeSpan TimeSpan::FromTotalSeconds(double value)
{
    return TimeSpan(value * kMicrosecondsPerSecond);
}

TimeSpan TimeSpan::FromTotalMinutes(double value)
{
    return TimeSpan(value * kMicrosecondsPerMinute);
}

TimeSpan TimeSpan::FromTotalHours(double value)
{
    return TimeSpan(value * kMicrosecondsPerHour);
}

TimeSpan TimeSpan::FromTotalDays(double value)
{
    return TimeSpan(value * kMicrosecondsPerDay);
}

TimeSpan TimeSpan::FromMilliseconds(int value)
{
    return TimeSpan(value * kMicrosecondsPerMillisecond);
}

TimeSpan TimeSpan::FromSeconds(int value)
{
    return TimeSpan(value * kMicrosecondsPerSecond);
}

TimeSpan TimeSpan::FromMinutes(int value)
{
    return TimeSpan(value * kMicrosecondsPerMinute);
}

TimeSpan TimeSpan::FromHours(int value)
{
    return TimeSpan(value * kMicrosecondsPerHour);
}

TimeSpan TimeSpan::FromDays(int value)
{
    return TimeSpan(value * kMicrosecondsPerDay);
}

TimeSpan::TimeSpan()
    : tick_(0)
{
}

TimeSpan::~TimeSpan()
{
}

TimeSpan::TimeSpan(int32_t tick)
    : tick_(static_cast<int64_t>(tick))
{
}

TimeSpan::TimeSpan(int64_t tick)
    : tick_(tick)
{
}

TimeSpan::TimeSpan(double tick)
    : tick_(static_cast<int64_t>(tick))
{
}

TimeSpan & TimeSpan::operator=(int64_t tick)
{
    tick_ = tick;
    return *this;
}

TimeSpan & TimeSpan::operator += (const TimeSpan & right)
{
    tick_ += right.tick_;
    return *this;
}

TimeSpan & TimeSpan::operator -= (const TimeSpan & right)
{
    tick_ -= right.tick_;
    return *this;
}

bool TimeSpan::operator == (const TimeSpan & right) const
{
    return std::abs(tick_) == std::abs(right.tick_);
}

bool TimeSpan::operator != (const TimeSpan & right) const
{
    return std::abs(tick_) != std::abs(right.tick_);
}

bool TimeSpan::operator < (const TimeSpan & right) const
{
    return std::abs(tick_) < std::abs(right.tick_);
}

bool TimeSpan::operator <= (const TimeSpan & right) const
{
    return std::abs(tick_) <= std::abs(right.tick_);
}

bool TimeSpan::operator > (const TimeSpan & right) const
{
    return std::abs(tick_) > std::abs(right.tick_);
}

bool TimeSpan::operator >= (const TimeSpan & right) const
{
    return std::abs(tick_) >= std::abs(right.tick_);
}

void TimeSpan::set_tick(int64_t tick)
{
    tick_ = tick;
}

int64_t TimeSpan::tick() const
{
    return tick_;
}

double TimeSpan::TotalMilliseconds() const
{
    return static_cast<double>(tick_) / kMicrosecondsPerMillisecond;
}

double TimeSpan::TotalSeconds() const
{
    return static_cast<double>(tick_) / kMicrosecondsPerSecond;
}

double TimeSpan::TotalMinutes() const
{
    return static_cast<double>(tick_) / kMicrosecondsPerMinute;
}

double TimeSpan::TotalHours() const
{
    return static_cast<double>(tick_) / kMicrosecondsPerHour;
}

double TimeSpan::TotalDays() const
{
    return static_cast<double>(tick_) / kMicrosecondsPerDay;
}

int32_t TimeSpan::Milliseconds() const
{
    int64_t ms = tick_ % kMicrosecondsPerSecond / kMicrosecondsPerMillisecond;
    return static_cast<int32_t>(ms);
}

int32_t TimeSpan::Seconds() const
{
    int64_t seconds =  tick_ % kMicrosecondsPerMinute / kMicrosecondsPerSecond;
    return static_cast<int32_t>(seconds);
}

int32_t TimeSpan::Minutes() const
{
    int64_t minutes = tick_ % kMicrosecondsPerHour / kMicrosecondsPerMinute;
    return static_cast<int32_t>(minutes);
}

int32_t TimeSpan::Hours() const
{
    int64_t hours = tick_ % kMicrosecondsPerDay / kMicrosecondsPerHour;
    return static_cast<int32_t>(hours);
}

int32_t TimeSpan::Days() const
{
    int64_t days = tick_ / kMicrosecondsPerDay;
    return static_cast<int32_t>(days);
}

TimeSpan operator + (const TimeSpan & left, const TimeSpan & right)
{
    TimeSpan ts(left.tick());
    ts += right;
    return ts;
}

TimeSpan operator - (const TimeSpan & left, const TimeSpan & right)
{
    TimeSpan ts(left.tick());
    ts -= right;
    return ts;
}

}
