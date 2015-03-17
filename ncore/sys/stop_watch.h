#ifndef NCORE_SYS_STOP_WATCH_H_
#define NCORE_SYS_STOP_WATCH_H_

#include <ncore/ncore.h>

namespace ncore
{


class StopWatch
{
public:
    StopWatch();

    void Start();
    void Stop();
    uint32_t ElapsedTime() const;
private:
    uint32_t start_tick_;
    uint32_t stop_tick_;
    bool stoped_;
};

}

#endif