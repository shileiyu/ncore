#ifndef NCORE_BASE_TIMESPAN_H_
#define NCORE_BASE_TIMESPAN_H_

#include <ncore/ncore.h>

namespace ncore
{


class TimeSpan
{
public:
    static const int32_t kHoursPerDay;
    static const int32_t kMinutesPerHour;
    static const int32_t kSecondsPerMinute;
    static const int32_t kMillisecondsPerSecond;
    static const int32_t kMicrosecondsPerMillisecond;
    static const int32_t kMicrosecondsPerSecond;
    static const int32_t kMicrosecondsPerMinute;
    static const int64_t kMicrosecondsPerHour;
    static const int64_t kMicrosecondsPerDay;

public:
    static TimeSpan FromTotalMilliseconds(double value);
    static TimeSpan FromTotalSeconds(double value);
    static TimeSpan FromTotalMinutes(double value);
    static TimeSpan FromTotalHours(double value);
    static TimeSpan FromTotalDays(double value);

    static TimeSpan FromMilliseconds(int32_t value);
    static TimeSpan FromSeconds(int32_t value);
    static TimeSpan FromMinutes(int32_t value);
    static TimeSpan FromHours(int32_t value);
    static TimeSpan FromDays(int32_t value);

public:
    TimeSpan();
    ~TimeSpan();

    TimeSpan(int32_t tick);
    TimeSpan(int64_t tick);
    TimeSpan(double tick);

    TimeSpan & operator = (int64_t tick);
    TimeSpan & operator += (const TimeSpan & right);
    TimeSpan & operator -= (const TimeSpan & right);

    bool operator == (const TimeSpan & right) const;
    bool operator != (const TimeSpan & right) const;
    bool operator < (const TimeSpan & right) const;
    bool operator <= (const TimeSpan & right) const;
    bool operator > (const TimeSpan & right) const;
    bool operator >= (const TimeSpan & right) const;

    void set_tick(int64_t tick);

    int64_t tick() const;

    double TotalMilliseconds() const;
    double TotalSeconds() const;
    double TotalMinutes() const;
    double TotalHours() const;
    double TotalDays() const;

    int32_t Milliseconds() const;
    int32_t Seconds() const;
    int32_t Minutes() const;
    int32_t Hours() const;
    int32_t Days() const;

private:
    int64_t tick_;  // microseconds
};

TimeSpan operator + (const TimeSpan & left, const TimeSpan & right);
TimeSpan operator - (const TimeSpan & left, const TimeSpan & right);


}

#endif