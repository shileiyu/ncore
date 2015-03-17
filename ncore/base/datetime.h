#ifndef NCORE_BASE_DATETIME_H_
#define NCORE_BASE_DATETIME_H_


#include "object.h"
#include "timespan.h"

namespace ncore
{

class Timezone
{
public:
    Timezone();
private:
    uint32_t bias_;
    const char * abbr_;
    const char * name_;
};
/*
DateTime represents an absolute point in timeline, 
internal represented as microseconds
*/
class DateTime
{
    friend class DateTimeUtils;
public:
    static const int64_t kInvalidTick;
    static const int64_t kWindowsEpochDeltaSeconds;
    static const int64_t kMillisecondsPerSecond;
    static const int64_t kMicrosecondsPerSecond;
public:
    static DateTime UTCNow();
    static DateTime FromUnixTime(time_t unix_time);
public:
    DateTime();
    DateTime(int64_t tick);
    ~DateTime();

    bool IsValid() const;

    void set_tick(int64_t tick);
    int64_t tick() const;

    template<typename Dst, size_t Size>
    inline size_t Format(Dst (&dst)[Size], const Dst * fmt) const
    {
        return Format(dst, Size, fmt);
    }

    size_t Format(char * str, size_t size, const char * fmt) const;

    size_t Format(wchar_t * str, size_t size, const wchar_t * fmt) const;

    time_t ToUnixTime() const;
    /*
    TODO: 
    + manipulation methods.
    + more operators.
    */
    bool operator == (const DateTime & right) const;
    bool operator != (const DateTime & right) const;
    TimeSpan operator - (const DateTime & right) const;

private:
    int64_t tick_;
    Timezone tz_;
};

}

#endif